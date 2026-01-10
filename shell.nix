{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
	packages = with pkgs; [
		gcc
		gnumake
		pkg-config
		sdl3
		sdl3-ttf
		glew
		glm
		clang-tools
		bear
	];
	shellHook = ''
  if [ -z "$LD_LIBRARY_PATH" ]; then
    export LD_LIBRARY_PATH="/run/opengl-driver/lib:/run/opengl-driver-32/lib"
  else
    export LD_LIBRARY_PATH="/run/opengl-driver/lib:/run/opengl-driver-32/lib:$LD_LIBRARY_PATH"
  fi
'';

}
