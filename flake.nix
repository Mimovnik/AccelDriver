{
  description = "Out-of-tree Linux kernel module development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
  };

  outputs = {
    self,
    nixpkgs,
  }: let
    system = "x86_64-linux"; # Adjust if you're on a different system
    pkgs = import nixpkgs {
      inherit system;
      config = {
        allowUnfree = true;
      };
    };
    kernel = pkgs.linuxPackages.kernel; # Replace with your desired kernel version
  in {
    # Package for building the kernel module
    packages.${system}.default = pkgs.stdenv.mkDerivation {
      name = "hello-module";
      src = ./.;
      buildInputs = with pkgs; [
        kernel.dev
        kernel.moduleBuildDependencies
        gcc
        clang
        pkg-config
        kmod
      ];
      KERNELDIR = "${kernel.dev}/lib/modules/${kernel.modDirVersion}/build";
      buildPhase = ''
        make -C $KERNELDIR M=$(pwd) modules
      '';
      installPhase = ''
        mkdir -p $out/lib/modules/${kernel.modDirVersion}/extra
        cp hello.ko $out/lib/modules/${kernel.modDirVersion}/extra/
      '';
    };

    # Development shell
    devShells.${system}.default = pkgs.mkShell {
      inputsFrom = [self.packages.${system}.default];

      nativeBuildInputs = [
        pkgs.just
      ];

      shellHook = ''
        export KERNELDIR=${kernel.dev}/lib/modules/${kernel.modDirVersion}/build
        echo
        echo "Kernel source directory set to $KERNELDIR"

        echo
        echo "Welcome to the dev shell!"
        echo "Use 'just <recipe>' to run Justfile recipes (commands). Use 'just -l' to show the below menu."
        just -l
      '';
    };
  };
}
