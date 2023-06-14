# Coding Style

To ensure a standardized code style, we use the Google C++ Style Guide.
CppStyle is our recommended Eclipse Plugin. It uses the Formatter [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and the Style Checker [cpplint.py](https://github.com/cpplint/cpplint).

## Setup pre-commit

See documentation of [pre-commit](https://pre-commit.com/) first for reference.
Install pre-commit if missing locally

```
pip3 install pre-commit
```

Configure pre-commit to activate all git-hooks defined in `pre-commit-config.yaml`. From now on all commit will be checked before the commit takes place.

## C++

### Installation

Within the omnetpp Docker container, [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and the Style Checker [cpplint.py](https://github.com/cpplint/cpplint) are already installed.

If you additionally want to guarantee that only correctly formatted code is committed, you can set-up a git pre-commit hook
which checks the files you are about to commit. A suitable script is available within the `scripts` subdirectory. Execute:

```
user@host:~/crownet$ scripts/git-format/git-pre-commit-format install
```

If your are using the (recommended) execution of the simulation based on the provided Docker containers, you are done now. In order to disable the style check for individual projects (for example containing legacy code or code of other projects not conforming to the style guide), go to
**Project properties -> C/C++ General -> Formatter** and disable the style checks.


If the omnetpp Docker container is *not* used, the style check tools can manually be installed by the commands in the following subsections.

#### Manual Installation of cpplint.py
```
pip3 install cpplint
```

#### Manual Installation of clang-format for UNIX systems

You can install it through the package manager

For example with Ubuntu:
```
sudo apt-get install clang-format
```
Other ways installing it are building it from source or extracting it from the LLVM toolchain
and copying the bin/clang-format into your PATH.


#### Manual Installation of clang-format for Windows

There is an installer for Windows: https://llvm.org/builds/
(untested)

### Configure CppStyle

To configure CppStyle globally, go to **Preferences -> C/C++ -> CppStyle** dialog.

To configure CppSytle for a C/C++ project, go to **Project properties -> CppStyle** dialog.

To enable CppStyle(clang-format) as default C/C++ code formatter, go to **Preferences -> C/C++ -> Code Style -> Formatter** page and switch **"Code Formatter"** from **[built-in]** to **"CppStyle (clang-format)"**

To enable CppStyle(clang-format) as C/C++ code formatter for a project, go to **Project properties -> C/C++ General -> Formatter** page and switch **"Code Formatter"** from **[built-in] **to **"CppStyle (clang-format)"**

### How to use CppStyle

#### Style Check with cpplint.py
By pressing the  **Run C/C++ Code Analysis** when you right-click a file, cpplint.py will check the file and display differences in the editor.
In the Properties of CppStyle an automatic analysis can be set to trigger when saving the file.

#### Formatting Code with clang-format
The whole file or marked code can be formated using the shortcut **Command + Shift + f** on MacOS or **Ctrl + Shift + f** on Linux and other systems.

Further information: https://github.com/wangzw/CppStyle, http://www.cppstyle.com/

## Python

Use [Black](https://github.com/psf/black) Python code formatter.

### Installation

```
pip install black
```
For IDE support see Black [Editor integration](https://black.readthedocs.io/en/stable/editor_integration.html).

