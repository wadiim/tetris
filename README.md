# Tetris

A terminal-based tetris game for GNU/Linux.

<p align="center">
  <img src="https://user-images.githubusercontent.com/33803413/220151227-54877461-c5be-45ed-aee4-b0b056968343.gif" />
</p>

## Building

Enter the following command to build an optimized binary:

```sh
make
```

In order to surpass any optimizations and generate a binary containing debug information, enter the following command:

```sh
make debug
```

## Usage

Enter the following command from the project's root directory to run the program:

```sh
./bin/tetris
```

### Key bindings

| Keystroke | Effect |
| --- | --- |
| <kbd>←</kbd> / <kbd>H</kbd> | Move tetromino left |
| <kbd>→</kbd> / <kbd>L</kbd> | Move tetromino right |
| <kbd>↓</kbd> / <kbd>J</kbd> | Rotate tetromino clockwise |
| <kbd>↑</kbd> / <kbd>K</kbd> | Rotate tetromino anticlockwise |
| <kbd>Spacebar</kbd> | Increase falling speed |
| <kbd>Enter</kbd> | Drop tetromino |
| <kbd>Q</kbd> | Quit |

## License

[MIT](https://github.com/wadiim/tetris/blob/main/LICENSE)
