#include <iostream>
#include <filesystem>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/ADT/Optional.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include "driver.hpp"
#include "jasper_number.hpp"

static bool createExecutableObject(Driver &driver) {
	// Initialize the target registry etc.
	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();

	auto targetTriple = llvm::sys::getDefaultTargetTriple();
	driver.module->setTargetTriple(targetTriple);

	std::string error;
	auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);

	// Print an error and exit if we couldn't find the requested target.
	// This generally occurs if we've forgotten to initialise the
	// TargetRegistry or we have a bogus target triple.
	if (!target) {
		llvm::errs() << error;
		return false;
	}

	auto cpu = "generic";
	auto features = "";

	llvm::TargetOptions opt;
	auto RM = llvm::Optional<llvm::Reloc::Model>();
	auto targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, RM);

	driver.module->setDataLayout(targetMachine->createDataLayout());

	std::filesystem::create_directories("./bin");

	if(![&]{
		auto filename = "bin/output.o";
		std::error_code EC;
		llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);

		if (EC) {
			llvm::errs() << "Could not open file: " << EC.message();
			return false;
		}

		llvm::legacy::PassManager pass;
		auto filetype = llvm::CGFT_AssemblyFile;

		if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, filetype)) {
			llvm::errs() << "targetMachine can't emit a file of this type";
			return false;
		}

		pass.run(*driver.module);
		dest.flush();
		return true;
	}()) return false;

	if(![&] {
		auto filename = "bin/assembly.s";
		std::error_code EC;
		llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);

		llvm::legacy::PassManager pass;
		auto filetype = llvm::CGFT_AssemblyFile;

		if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, filetype)) {
			llvm::errs() << "targetMachine can't emit a file of this type";
			return false;
		}

		pass.run(*driver.module);
		dest.flush();
		return true;
	}()) return false;

	return true;
}

int main (int argc, char *argv[]) {
	int res = 0;
	Driver driver;
	for (int i = 1; i < argc; ++i) {
		if (argv[i] == std::string("-p")) {
			driver.trace_parsing = true;
		} else if (argv[i] == std::string("-s")) {
			driver.trace_scanning = true;
		} else if (!driver.parse(argv[i])) {
			for (const auto & variable : driver.variables) {
				std::cout << variable.first << ": " << variable.second << '\n';
			}
			for (const auto & variable : driver.string_variables) {
				std::cout << variable.first << ": " << variable.second << '\n';
			}

			createExecutableObject(driver);

			driver.module->print(llvm::errs(), nullptr);
		} else {
			res = 1;
		}
	}
	return res;
}
