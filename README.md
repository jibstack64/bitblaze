# bitblaze

![GitHub tag (latest by date)](https://img.shields.io/github/v/tag/jibstack64/bitblaze) ![GitHub Workflow Status (with event)](https://img.shields.io/github/actions/workflow/status/jibstack64/bitblaze/c-cpp.yml)
 ![GitHub License](https://img.shields.io/github/license/jibstack64/bitblaze) 

An esoteric programming language developed during mid-class boredom.

Type `bitblaze -h` for a list of possible arguments.

[Example scripts](https://github.com/jibstack64/bitblaze/tree/main/examples)

| **Key** | **Function**                                                                                                                                                                                                                                      | **Cursor movement** |
|---------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------|
|    v    | Represents a 0 bit.                                                                                                                                                                                                                               |          0          |
|    ^    | Represents a 1 bit.                                                                                                                                                                                                                               |          0          |
|    <    | Moves the cursor to the left.                                                                                                                                                                                                                     |          -1         |
|    >    | Moves the cursor to the right.                                                                                                                                                                                                                    |          +1         |
|    ~    | Changes the cursor position to the value of the current cell.                                                                                                                                                                                     |   =[current cell]   |
|    +    | Adds 1 to the value of the current cell.                                                                                                                                                                                                          |          0          |
|    -    | Subtracts 1 from the value of the current cell.                                                                                                                                                                                                   |          0          |
|    [    | Opens a loop. Code loops until the value of the current cell is 0.                                                                                                                                                                                |          0          |
|    ]    | Closes a loop.                                                                                                                                                                                                                                    |          0          |
|    (    | Opens an auto-loop. This loop automatically subtracts 1 from the cell it is instanciated at for every loop until its value reaches 0.                                                                                                             |          0          |
|    )    | Closes a loop.                                                                                                                                                                                                                                    |          0          |
|    {    | Opens a function block. The function data is stored seperately and assigned an ID. This ID is automatically assigned to the currently selected cell.                                                                                              |          0          |
|    ;    | Can be used interchangably with `]`, `)` and `}` to end a block. If outside of a block, ends the program.                                                                                                                                         |          0          |
|    #    | Shifts through every character in the buffer from the cell it is called at and prints each cell value until a null (0) byte is reached. Does NOT change the cursor position.                                                                      |          0          |
|    ?    | Waits for a single character input from the user and stores it in the current cell.                                                                                                                                                               |          0          |
|    *    | Runs the function that matches the ID the current cell holds, if any.                                                                                                                                                                             |          0          |
|    "    | Open/close a string. When you declare a string, the bytes within it are spread across the cells following that of which you declared the string at. The current cell position is shifted to the one that houses the last character of the string. |   +[string length]  |
