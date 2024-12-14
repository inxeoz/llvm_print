; this .ll program print content of file in terminal './program filename.filetension'
; but it needs to be compiled though static compiled

declare i8* @fopen(i8*, i8*)
declare i32 @printf(i8*, ...)
declare i32 @fgetc(i8*)

@mode = private constant [2 x i8] c"r\00"
@err_msg = private constant [19 x i8] c"File open failed!\0A\00"
@fmt_char = private constant [3 x i8] c"%c\00"

define i32 @main(i32 %argc, i8** %argv) {
entry:
  ; Check if a filename is provided
  %arg_check = icmp sgt i32 %argc, 1
  br i1 %arg_check, label %open_file, label %error

open_file:
  ; Load the filename from argv[1]
  %filename_ptr = getelementptr inbounds i8*, i8** %argv, i32 1
  %filename = load i8*, i8** %filename_ptr

  ; Call fopen with the filename
  %file = call i8* @fopen(i8* %filename,
                         i8* getelementptr inbounds ([2 x i8], [2 x i8]* @mode, i32 0, i32 0))

  ; Check if file is null
  %file_check = icmp eq i8* %file, null
  br i1 %file_check, label %error, label %read_loop

read_loop:
  ; Allocate space for one character
  %char_ptr = alloca i8
  br label %loop

loop:
  ; Read a character from the file
  %read = call i32 @fgetc(i8* %file)
  %char = trunc i32 %read to i8
  store i8 %char, i8* %char_ptr

  ; Check for EOF (-1)
  %eof_check = icmp eq i32 %read, -1
  br i1 %eof_check, label %done, label %print_char

print_char:
  ; Print the character
  %loaded_char = load i8, i8* %char_ptr
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @fmt_char, i32 0, i32 0), i8 %loaded_char)
  br label %loop

error:
  ; Print an error message
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @err_msg, i32 0, i32 0))
  ret i32 1

done:
  ; End the program successfully
  ret i32 0
}
