../TripleWitchHut 1.12 181201211981019340 100000
diff out.txt test.txt
diff -s ../out.txt test.txt > /dev/null
if [ $? -ne 0 ]; then
    echo "Test Failed"
fi