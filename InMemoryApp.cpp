#include <iostream>
#include <fstream>

#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/FrontendTool/Utils.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include "Perf.h"

using namespace clang;
using namespace clang::driver;
using namespace llvm::vfs;

// TODO pass as a command line argument
#if defined(__unix__)
#define CLANG_PATH                                                             \
  "/home/dbhaskar/dockerx/git/amd-stg-open/lightning/build/compiler/"          \
  "llvm-project/bin/clang++"
#else
#define CLANG_PATH "C:\\code\\llvm-project\\build\\bin\\clang++"
#endif

class FileSystemAdaptor {
public:
  virtual void setInputFileSystem(CompilerInstance *CI) = 0;
  virtual void setOutputFileSystem(CompilerInstance *CI) = 0;
  virtual void showFS() = 0;
};

class ProxyFS : public FileSystemAdaptor {
public:
  void setInputFileSystem(CompilerInstance *CI) override {}
  void setOutputFileSystem(CompilerInstance *CI) override {}
  void showFS() override{};

  ProxyFS() : FileSystemAdaptor() {}
};

#define UPSTREAM_PATCH
// #define IN_HOUSE_PATCH

#ifdef IN_HOUSE_PATCH
class InMemoryFS : public FileSystemAdaptor {
public:
  InMemoryFS(llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> FS)
      : VFS(FS) {
    assert(VFS);
    VFS->setCurrentWorkingDirectory("/");
  }

  void setInputFileSystem(CompilerInstance *CI) override {
    CI->createFileManager(VFS);
  }

  void setOutputFileSystem(CompilerInstance *CI) override {
    CI->setOutputFileSystem(VFS);
  }

  void showFS() override { std::cout << VFS->toString(); };

private:
  llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> VFS;
};
#elif defined UPSTREAM_PATCH
class InMemoryFS : public FileSystemAdaptor {
public:
  InMemoryFS(llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> FS)
      : VFS(FS) {
    assert(VFS);
    VFS->setCurrentWorkingDirectory("/");
  }

  void setInputFileSystem(CompilerInstance *CI) override {
    CI->createFileManager(VFS);
  }

  void setOutputFileSystem(CompilerInstance *CI) override {
    llvm::IntrusiveRefCntPtr<InMemoryOutputBackend> Backend =
        llvm::makeIntrusiveRefCnt<InMemoryOutputBackend>(VFS);
    CI->setOutputBackend(Backend);
  }

  void showFS() override { std::cout << VFS->toString(); };

private:
  llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> VFS;
};
#endif

class TestCompilerInvocation {

public:
  void EmitBC(const char *Source, const char *Output, Profile &P) {
    SmallVector<const char *, 16> Args;
    Args.clear();
    Args.push_back("-cc1");
    Args.push_back("-emit-llvm");
    Args.push_back("-o");
    Args.push_back(Output);
    Args.push_back("-c");
    Args.push_back(Source);
    prepareArgs(CLANG_PATH, Args);
    InvokeCompilerInstance(P, false);
  }

  void CompileBC(const char *Source, const char *Output, Profile &P) {
    SmallVector<const char *, 16> Args;
    Args.clear();
    Args.push_back("-cc1");
    Args.push_back("-o");
    Args.push_back(Output);
    Args.push_back("-c");
    Args.push_back(Source);
    Args.push_back(nullptr);
    prepareArgs(CLANG_PATH, Args);
    InvokeCompilerInstance(P, true);
  }

  TestCompilerInvocation(std::unique_ptr<FileSystemAdaptor> InputFS)
      : VFS(std::move(InputFS)) {
    prepareDiagnostics();
  }

private:
  void prepareDiagnostics() {
    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
    DiagClient = new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
    Diags = new DiagnosticsEngine(&*DiagID, &*DiagOpts, DiagClient);
  }

  // TODO Find a better way to get the arguments.
  void prepareArgs(const char *ClangPath, SmallVector<const char *, 16> Args) {
    const std::unique_ptr<Driver> ClangDriver(
        new Driver(ClangPath, llvm::sys::getDefaultTargetTriple(), *Diags));
    const std::unique_ptr<Compilation> Build(
        ClangDriver->BuildCompilation(llvm::ArrayRef<const char *>(Args)));
    JobList &Jobs = Build->getJobs();

    Command *Cmd = cast<Command>(&*Jobs.begin());
    if (strcmp(Cmd->getCreator().getName(), "clang")) {
      std::cerr << "Name not clang"
                << "\n";
      exit(1);
    }

    Argv.clear();
    for (const auto &Arg : Cmd->getArguments()) {
      // std::cout << Arg << "\n";
      Argv.push_back(Arg);
    }
  }

  std::unique_ptr<CompilerInstance> createCompilerInstance() {
    std::unique_ptr<CompilerInstance> CI(new CompilerInstance());

    SmallVector<const char *, 128> Args;
    Args.clear();
    for (const auto &argv : Argv)
      Args.push_back(argv.c_str());
    Args.push_back(nullptr);

    auto Success =
        CompilerInvocation::CreateFromArgs(CI->getInvocation(), Args, *Diags);
    if (!Success)
      llvm_unreachable("CreateFromArgs failed");

    CI->createDiagnostics(DiagClient, /* ShouldOwnClient */ false);
    if (!CI->hasDiagnostics())
      llvm_unreachable("createDiagnostics failed");

    return std::move(CI);
  }

  void InvokeCompilerInstance(Profile &P, bool SetInputFS = false) {
    std::unique_ptr<CompilerInstance> CI = createCompilerInstance();
    if (SetInputFS)
      VFS->setInputFileSystem(CI.get());

    // Set the output always.
    VFS->setOutputFileSystem(CI.get());

    P.Start();
    if (!ExecuteCompilerInvocation(CI.get()))
      llvm_unreachable("ExecuteCompilerInvocation failed");
    P.Stop();
  }

  TextDiagnosticPrinter *DiagClient = nullptr;
  DiagnosticsEngine *Diags = nullptr;

  SmallVector<std::string, 128> Argv;
  std::unique_ptr<FileSystemAdaptor> VFS;
};

#if defined(UPSTREAM_PATCH) || defined(IN_HOUSE_PATCH)
static void DumpFile(IntrusiveRefCntPtr<InMemoryFileSystem> FS,
                     const char *Source, const char *Output) {
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> BufferOrErr =
      FS->getBufferForFile(Source);
  if (!BufferOrErr) {
    std::cerr << "getBufferForFile failed\n";
    exit(1);
  }
  llvm::MemoryBuffer &Buffer = *BufferOrErr.get();

  std::ofstream DumpFile(Output, std::ios::out | std::ios::binary);
  DumpFile.write(Buffer.getBufferStart(), Buffer.getBufferSize());
  DumpFile.close();
}
#endif

int main(int argc, char **argv) {
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllTargets();
  llvm::InitializeAllAsmPrinters();
  llvm::InitializeAllAsmParsers();

  Profile P1("EmitBC"), P2("CompileBC");
  TestCompilerInvocation T1(std::make_unique<ProxyFS>());
  for (int i = 0; i < 10; i++) {
    T1.EmitBC("test.cpp", "test.bc", P1);
    T1.CompileBC("test.bc", "test.o", P2);
  }

#if defined(UPSTREAM_PATCH) || defined(IN_HOUSE_PATCH)
  Profile P3("EmitBC_InMemory"), P4("CompileBC_InMemory");
  for (int i = 0; i < 10; i++) {
    IntrusiveRefCntPtr<InMemoryFileSystem> IMFS =
        llvm::makeIntrusiveRefCnt<InMemoryFileSystem>();
    TestCompilerInvocation T2(std::make_unique<InMemoryFS>(IMFS));
    T2.EmitBC("test.cpp", "test.bc", P3);
    T2.CompileBC("test.bc", "test.o", P4);

    if (i == 0) {
      DumpFile(IMFS, "test.bc", "dump.bc");
      DumpFile(IMFS, "test.o", "dump.o");
    }
  }
#endif

  llvm::llvm_shutdown();

  return 0;
}
