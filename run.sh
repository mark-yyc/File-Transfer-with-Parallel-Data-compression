make
mkdir bin
echo "Generate random text file"
./generateFile
echo "Start test on compression and decompression:"
./test_compress_decompress
echo "Check consistency between output file and original file:"
./md5check.sh ./data/english_text_bible.txt ./bin/output.txt
echo "Start test on data compression in different number of thread:"
./test_data_compression
echo "Finish test, clean source and bin file"
make clean
rm -rf bin
rm -f ./data/random_text.txt