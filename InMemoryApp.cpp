#include <iostream>

#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/FrontendTool/Utils.h>
#include "llvm/Support/FileSystem.h"
#include <llvm/Support/Host.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include "llvm/Support/TargetRegistry.h"
#include <llvm/Support/TargetSelect.h>

#include "Perf.h"

using namespace clang;
using namespace clang::driver;
using namespace llvm::vfs;

void setArgsForCompileBC(SmallVector<const char *, 128> &Argv, const char *Source,
    const char *output) {
  Argv.clear();
  Argv.push_back("-cc1");
  Argv.push_back("-triple");
  Argv.push_back("x86_64-unknown-linux-gnu");
  Argv.push_back("-emit-obj");
  Argv.push_back("-mrelax-all");
  Argv.push_back("--mrelax-relocations");
  Argv.push_back("-disable-free");
  Argv.push_back("-main-file-name");
  Argv.push_back(Source);
  Argv.push_back("-mrelocation-model");
  Argv.push_back("static");
  Argv.push_back("-mframe-pointer=all");
  Argv.push_back("-fmath-errno");
  Argv.push_back("-fno-rounding-math");
  Argv.push_back("-mconstructor-aliases");
  Argv.push_back("-munwind-tables");
  Argv.push_back("-target-cpu");
  Argv.push_back("x86-64");
  Argv.push_back("-tune-cpu");
  Argv.push_back("generic");
  Argv.push_back("-fno-split-dwarf-inlining");
  Argv.push_back("-debugger-tuning=gdb");
  Argv.push_back("-resource-dir");
  Argv.push_back("/home/dbhaskar/dockerx/git/workspace-2/lightning/build/"
                 "compiler/llvm-project/lib/clang/13.0.0");
  Argv.push_back("-fdebug-compilation-dir");
  Argv.push_back("/home/dbhaskar/dockerx/git/personal/LLVM-Internals/vfs/"
                 "build.workspace-2");
  Argv.push_back("-ferror-limit");
  Argv.push_back("19");
  Argv.push_back("-fgnuc-version=4.2.1");
  Argv.push_back("-fcolor-diagnostics");
  Argv.push_back("-faddrsig");
  Argv.push_back("-o");
  Argv.push_back(output);
  Argv.push_back("-x");
  Argv.push_back("ir");
  Argv.push_back(Source);
  Argv.push_back(nullptr);
}

void setArgsForEmitBC(SmallVector<const char *, 128> &Argv, const char *Source,
    const char *output) {
  Argv.clear();
  Argv.push_back("-cc1");
  Argv.push_back("-triple");
  Argv.push_back("x86_64-unknown-linux-gnu");
  Argv.push_back("-emit-llvm-bc");
  Argv.push_back("-emit-llvm-uselists");
  Argv.push_back("-disable-free");
  Argv.push_back("-main-file-name");
  Argv.push_back(Source);
  Argv.push_back("-mrelocation-model");
  Argv.push_back("static");
  Argv.push_back("-mframe-pointer=all");
  Argv.push_back("-fmath-errno");
  Argv.push_back("-fno-rounding-math");
  Argv.push_back("-mconstructor-aliases");
  Argv.push_back("-munwind-tables");
  Argv.push_back("-target-cpu");
  Argv.push_back("x86-64");
  Argv.push_back("-tune-cpu");
  Argv.push_back("generic");
  Argv.push_back("-fno-split-dwarf-inlining");
  Argv.push_back("-debugger-tuning=gdb");
  Argv.push_back("-resource-dir");
  Argv.push_back("/home/dbhaskar/dockerx/git/workspace-2/lightning/build/"
                 "compiler/llvm-project/lib/clang/13.0.0");
  Argv.push_back("-internal-isystem");
  Argv.push_back(
      "/usr/lib/gcc/x86_64-linux-gnu/7.5.0/../../../../include/c++/7.5.0");
  Argv.push_back("-internal-isystem");
  Argv.push_back("/usr/lib/gcc/x86_64-linux-gnu/7.5.0/../../../../include/"
                 "x86_64-linux-gnu/c++/7.5.0");
  Argv.push_back("-internal-isystem");
  Argv.push_back("/usr/lib/gcc/x86_64-linux-gnu/7.5.0/../../../../include/"
                 "x86_64-linux-gnu/c++/7.5.0");
  Argv.push_back("-internal-isystem");
  Argv.push_back("/usr/lib/gcc/x86_64-linux-gnu/7.5.0/../../../../include/c++/"
                 "7.5.0/backward");
  Argv.push_back("-internal-isystem");
  Argv.push_back("/usr/local/include");
  Argv.push_back("-internal-isystem");
  Argv.push_back("/home/dbhaskar/dockerx/git/workspace-2/lightning/build/"
                 "compiler/llvm-project/lib/clang/13.0.0/include");
  Argv.push_back("-internal-externc-isystem");
  Argv.push_back("/usr/include/x86_64-linux-gnu");
  Argv.push_back("-internal-externc-isystem");
  Argv.push_back("/include");
  Argv.push_back("-internal-externc-isystem");
  Argv.push_back("/usr/include");
  Argv.push_back("-fdeprecated-macro");
  Argv.push_back("-fdebug-compilation-dir");
  Argv.push_back("/home/dbhaskar/dockerx/git/personal/LLVM-Internals/vfs/"
                 "build.workspace-2");
  Argv.push_back("-ferror-limit");
  Argv.push_back("19");
  Argv.push_back("-fgnuc-version=4.2.1");
  Argv.push_back("-fcxx-exceptions");
  Argv.push_back("-fexceptions");
  Argv.push_back("-fcolor-diagnostics");
  Argv.push_back("-faddrsig");
  Argv.push_back("-o");
  Argv.push_back(output);
  Argv.push_back("-x");
  Argv.push_back("c++");
  Argv.push_back(Source);

  Argv.push_back(nullptr);
}

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
  void showFS() override {};

  ProxyFS() : FileSystemAdaptor() {}
};

// #define UPSTREAM_PATCH
//#define IN_HOUSE_PATCH

#ifdef IN_HOUSE_PATCH
class InMemoryFS : public FileSystemAdaptor {
public:
  InMemoryFS(llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> FS) : VFS(FS) {
    assert(VFS);
    VFS->setCurrentWorkingDirectory("/");
  }

  void setInputFileSystem(CompilerInstance *CI) override {
    CI->createFileManager(VFS);
  }

  void setOutputFileSystem(CompilerInstance *CI) override {
    CI->setOutputFileSystem(VFS);
  }

  void showFS() override {
    std::cout << VFS->toString();
  };
private:
  llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> VFS;
};
#elif defined UPSTREAM_PATCH
class InMemoryFS : public FileSystemAdaptor {
public:
  InMemoryFS(llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> FS) : VFS(FS) {
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

  void showFS() override {
    std::cout << VFS->toString();
  };

private:
  llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> VFS;
};
#endif

class TestCompilerInvocation {

public:
  void InvokeCI(Profile &P, bool SetInputFS = false) {
    std::unique_ptr<CompilerInstance> CI = CreateCI();
    if (SetInputFS)
      VFS->setInputFileSystem(CI.get());

    // Set the output always.
    VFS->setOutputFileSystem(CI.get());

    P.Start();
    if (!ExecuteCompilerInvocation(CI.get()))
      llvm_unreachable("ExecuteCompilerInvocation failed");
    P.Stop();
  }

  void EmitBC(const char *Source, const char *Output, Profile &P) {
    setArgsForEmitBC(Argv, Source, Output);
    InvokeCI(P, false);
  }

  void CompileBC(const char *Source, const char *Output, Profile &P) {
    setArgsForCompileBC(Argv, Source, Output);
    InvokeCI(P, true);
  }

  TestCompilerInvocation(std::unique_ptr<FileSystemAdaptor> InputFS) : VFS(std::move(InputFS)) {}

private:
  SmallVector<const char *, 128> Argv;
  std::unique_ptr<FileSystemAdaptor> VFS;

  std::unique_ptr<CompilerInstance> CreateCI() {
    // The clang driver needs a DiagnosticsEngine so it can report problems
    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
    TextDiagnosticPrinter *DiagClient =
        new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
    DiagnosticsEngine *Diags =
        new DiagnosticsEngine(&*DiagID, &*DiagOpts, DiagClient);

    std::unique_ptr<CompilerInstance> CI(new CompilerInstance());
    auto Success =
        CompilerInvocation::CreateFromArgs(CI->getInvocation(), Argv, *Diags);
    if (!Success)
      llvm_unreachable("CreateFromArgs failed");
    CI->createDiagnostics(DiagClient, /* ShouldOwnClient */ false);
    if (!CI->hasDiagnostics())
      llvm_unreachable("createDiagnostics failed");

    return std::move(CI);
  }
};

int main(int argc, char **argv) {
  std::cout << "Starting ----" << std::endl;

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
    T2.CompileBC("/test.bc", "/test.o", P4);
  }
#endif

  std::cout << "Done ----" << std::endl;

  llvm::llvm_shutdown();

  return 0;
}
