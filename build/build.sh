make -j4 &&

# echo all executables
echo &&
find . -type f -executable ! -name "*.*"
