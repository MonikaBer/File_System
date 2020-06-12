#include <vector>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include "SimpleFS.hpp"
#include <string>

BOOST_AUTO_TEST_SUITE(SimpleFS_test)

    SimpleFS fs("../tests/etc_test/simplefs_test.conf");

    BOOST_AUTO_TEST_CASE(CreateDuplicateFile)
    {
        if(fs.create("/test", 0) == 0)
            BOOST_CHECK_EQUAL(fs.create("/test", 0), -1);
        else
            BOOST_CHECK_EQUAL(fs._open("/test", 1), 0);
    }

    BOOST_AUTO_TEST_CASE(WrongPathCreate){
        BOOST_CHECK_EQUAL(fs.create("/test/abc1/test", 0), -1);
        if(fs.create("/dir", 1) == 0)
            BOOST_CHECK_EQUAL(fs.create("/dir/abc/test", 0), -1);
    }

    BOOST_AUTO_TEST_CASE(PathNotFromRoot){
        BOOST_CHECK_EQUAL(fs.create("aaa", 0), -1);
        BOOST_CHECK_EQUAL(fs.create("aaa/zcz", 0), -1);
        BOOST_CHECK_EQUAL(fs.create("~aaa/zcz", 0), -1);
    }

    BOOST_AUTO_TEST_CASE(OpenNotCreatedFile){
        BOOST_CHECK_EQUAL(fs._open("/not_a_file", 1), -1);
    }

    BOOST_AUTO_TEST_CASE(OpenWriteLockedFile){
        if(fs.create("/new_file", 0) == 0);
            BOOST_CHECK(fs._open("/new_file", 1) >= 0);
        BOOST_CHECK_EQUAL(fs._open("/new_file", 1), -1);
    }

    BOOST_AUTO_TEST_CASE(WriteRead){
        fs.create("/read_write", 0);
        int rw = fs._open("/read_write", 1);
        char write[16] = "TEST WRITE-READ";
        char read[16] = {0};
        BOOST_CHECK_EQUAL(fs._write(rw, write, 16), 16);
        BOOST_CHECK_EQUAL(fs._read(rw, read, 16), 16);
        BOOST_CHECK_EQUAL(strcmp(write, read), 0);
    }

    BOOST_AUTO_TEST_CASE(ReadTooMuch){
        fs.create("/read_too_much", 0);
        int rtm = fs._open("/read_too_much", 1);
        char write[17] = "TEST ReadTooMuch";
        char read[40] = {0};
        fs._write(rtm, write, 17);
        BOOST_CHECK_EQUAL(fs._read(rtm, read, 40), 17);
    }

    BOOST_AUTO_TEST_CASE(ReadNegative){
        fs.create("/read_negative", 0);
        int rtm = fs._open("/read_negative", 1);
        char write[18] = "TEST ReadNegative";
        char read[40] = {0};
        fs._write(rtm, write, 18);
        BOOST_CHECK_EQUAL(fs._read(rtm, read, -5), -1);
    }

    //whence : 0-beginning, 1-current, 2-end
    BOOST_AUTO_TEST_CASE(Lseek){
        fs.create("/lseek", 0);
        int ls = fs._open("/lseek", 1);
        char write[11] = "0123456789";
        char read[1] = {0};
        fs._write(ls, write, 11);
        BOOST_CHECK_EQUAL(fs._lseek(ls, 0, 5), 5);
        fs._read(ls, read, 1);
        BOOST_CHECK_EQUAL(read[0], write[5]);
        BOOST_CHECK_EQUAL(fs._lseek(ls, 1, -1), 4);
        fs._read(ls, read, 1);
        BOOST_CHECK_EQUAL(read[0], write[4]);
        BOOST_CHECK_EQUAL(fs._lseek(ls, 2, -1), 9);
        fs._read(ls, read, 1);
        BOOST_CHECK_EQUAL(read[0], write[9]);
    }
    //7 plikow
    BOOST_AUTO_TEST_CASE(UnlinkNotCreated){
        BOOST_CHECK_EQUAL(fs.unlink("/not_a_file"), -1);
    }

    BOOST_AUTO_TEST_CASE(RmdirRegularFile){
        fs.create("/regular", 0);
        BOOST_CHECK_EQUAL(fs.rmdir("/regular"), -2);
    }

    BOOST_AUTO_TEST_CASE(DuplicateNameMakeDir){
        fs.mkdir("/dir_1");
        BOOST_CHECK_EQUAL(fs.mkdir("/dir_1"), -1);
    }

    BOOST_AUTO_TEST_CASE(TooMuchFiles){
        std::string name = "./namex";
        int i = 0;
        for(; i<40; ++i){
            name[7] = i;
            if(fs.create(std::string(name), 0) < 0)
                break;
        }
        BOOST_CHECK(i <= 16);
        BOOST_CHECK_EQUAL(fs.create("/another_one", 0), -1);
        for(; i>=0; --i){
            name[7] = i;
            fs.unlink(std::string(name));
        }
    }

BOOST_AUTO_TEST_SUITE_END()

