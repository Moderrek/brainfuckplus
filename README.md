# Brainfuck+ Interpreter

Brainfuck+ is an **extended** version of the classic [Brainfuck](https://en.wikipedia.org/wiki/Brainfuck) language,  
featuring additional functionalities such as the `--memdump` and `--warning` flags, as well as debugging and analysis tools. This interpreter allows you to run programs written in BrainfuckPlus, providing useful options for debugging and inspecting program execution.

```bf
;>;       read two bytes as int
[-<+>]<   perform add
:         print result as int
```

## Commands

BrainfuckPlus uses the same basic commands as Brainfuck, with potential for extensions. Here are the core commands:

| Command | Description                                                                       |
|---------|-----------------------------------------------------------------------------------|
| `>`     | Move the memory pointer to the right                                              |
| `<`     | Move the memory pointer to the left                                               |
| `+`     | Increment the byte at the memory pointer                                          |
| `-`     | Decrement the byte at the memory pointer                                          |
| `.`     | Output the byte at the memory pointer (usually as an ASCII character)             |
| `:`     | Output the int at the memory pointer                                              |
| `,`     | Accept one byte of input, storing its value at the memory pointer                 |
| `;`     | Accept int in range <0; 255>, storing its value at the memory pointer             |
| `[`     | Jump forward to the command after the corresponding `]` if the byte at the memory pointer is 0 |
| `]`     | Jump back to the command after the corresponding `[` if the byte at the memory pointer is nonzero |
| `$`     | Stops execution of script                                                         |

## Features

- **`--memdump`**: Displays a memory dump after program execution.
- **`--warning`**: Shows warnings if potential issues are detected during execution.
- **`--version`**: Displays the interpreter version.

## Usage

To run a BrainfuckPlus program, use the following command:
```bash
./brainfuckplus [options] <file>...
```

### Example:

- Running a program without any flags:
  ```bash
  ./brainfuckplus program.bf
  ```

- Displaying a memory dump after program execution:
  ```bash
  ./brainfuckplus --memdump program.bf
  ```

## Flags

| Flag          | Description                                                             |
|---------------|-------------------------------------------------------------------------|
| `--memdump`   | Displays a memory dump after program execution                          |
| `--warning`   | Shows warnings during execution                                         |
| `--version`   | Displays the version of the interpreter                                 |
| `--help`      | Displays the possible flags                                             |

## Contributing

Feel free to submit bug reports or feature requests via the [Issues](https://github.com/Moderrek/brainfuckplus/issues) tab.
