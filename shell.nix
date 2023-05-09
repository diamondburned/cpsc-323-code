{ systemPkgs ? import <nixpkgs> {} }:

let lib  = systemPkgs.lib;
	pkgs = systemPkgs;

	# clangd hack.
	llvmPackages = pkgs.llvmPackages_latest;
	clang-unwrapped = llvmPackages.clang-unwrapped;
	clang  = llvmPackages.clang;
	clangd = pkgs.writeScriptBin "clangd" ''
	    #!${pkgs.stdenv.shell}
		export CPATH="$(${clang}/bin/clang -E - -v <<< "" \
			|& ${pkgs.gnugrep}/bin/grep '^ /nix' \
			|  ${pkgs.gawk}/bin/awk 'BEGIN{ORS=":"}{print substr($0, 2)}' \
			|  ${pkgs.gnused}/bin/sed 's/:$//')"
		export CPLUS_INCLUDE_PATH="$(${clang}/bin/clang++ -E - -v <<< "" \
			|& ${pkgs.gnugrep}/bin/grep '^ /nix' \
			|  ${pkgs.gawk}/bin/awk 'BEGIN{ORS=":"}{print substr($0, 2)}' \
			|  ${pkgs.gnused}/bin/sed 's/:$//')"
	    ${clang-unwrapped}/bin/clangd
	'';

	mkshell = pkgs.mkShell.override {
		stdenv = pkgs.gcc12Stdenv;
	};

	PROJECT_ROOT   = builtins.toString ./.;
	PROJECT_SYSTEM = pkgs.system;

in mkshell {
	# Poke a PWD hole for our shell scripts to utilize.
	inherit PROJECT_ROOT PROJECT_SYSTEM;

	buildInputs = with pkgs; [
		automake
		autoconf
		curl
		gdb
		git
		objconv
		go
		clang-analyzer
	] ++ [
		clangd
		clang
	];

	hardeningDisable = [ "fortify" ];
}
