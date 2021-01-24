for filename in `find . -name "*.[ch]"`;
    do echo "formatting file :" $filename && clang-format -i -style=file $filename;
done;
