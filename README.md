# ShellSim Project

## Overview

ShellSim is a Linux shell simulator project that replicates basic shell functionalities, providing users with a command-line interface to interact with their system. It aims to offer a user-friendly experience while showcasing fundamental shell features.

## Table of Contents

- [Overview](#overview)
- [Table of Contents](#table-of-contents)
- [Installation](#installation)
- [Usage](#usage)
- [Features](#features)
- [Contributing](#contributing)


## Installation

To install ShellSim, follow these simple steps:

```bash
# Clone the repository
git clone https://github.com/yuvalshmaryahu/ShellSim.git

# Change into the project directory
cd ShellSim

# Execute the following compile command:
gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 shell.c shellsim.c

## Usage

Use ShellSim just like a regular shell with added features. Here's a quick example:

```bash
# Run the shell
./shellsim

# Example commands
$ sleep 10
$ sleep 10 &
$ cat foo.txt | grep bar
$ cat < file.txt
$ cat foo > file.txt
```

## Features

ShellSim comes with several features to enhance your shell experience:

- **Custom Commands:** Easily extend the shell with your custom commands.
- **Scripting Support:** Write and execute shell scripts seamlessly.

## Contributing

We welcome contributions to enhance ShellSim. Follow these steps to contribute:

1. Fork the repository
2. Create a new branch (`git checkout -b feature/new-feature`)
3. Make your changes and commit them (`git commit -am 'Add new feature'`)
4. Push to the branch (`git push origin feature/new-feature`)
5. Open a pull request
