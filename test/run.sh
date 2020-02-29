../WitchHutFinder 1.14 1 100000 2
diff out.txt test.txt
diff -s out.txt test.txt > /dev/null
if [ $? -ne 0 ]; then
    echo "Test Failed"
fi