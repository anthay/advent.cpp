/*
    Will Crowther's original FORTRAN IV "Colossal Cave Adventure"
    recoded in non-idiomatic C++ for my own interest -- more literal
    than literate -- as faithful as I could make it.
    Anthony C. Hay, Devon, UK, May 2024
    https://github.com/anthay/Adventure

    "Colossal Cave Adventure (also known as Adventure or ADVENT) is a
    text-based adventure game, released in 1976 by developer Will Crowther
    for the PDP-10 mainframe computer. It was expanded upon in 1977 by
    Don Woods. In the game, the player explores a cave system rumored
    to be filled with treasure and gold. The game is composed of dozens
    of locations, and the player moves between these locations and
    interacts with objects in them by typing one- or two-word commands
    which are interpreted by the game's natural language input system.
    The program acts as a narrator, describing the player's location and
    the results of the player's attempted actions. It is the first
    well-known example of interactive fiction, as well as the first
    well-known adventure game, for which it was also the namesake."
    -- https://en.wikipedia.org/wiki/Colossal_Cave_Adventure
       Accessed 21 May, 2024

    "Crowther's original source code, which had been presumed lost for
    decades, was recovered in 2005 from a backup of Don Woods's student
    account at the Stanford Artificial Intelligence Lab (SAIL). The
    recovered files, dated March 1977, and bearing the in-game message
    “WELCOME TO ADVENTURE!!”, confirm that Crowther's original was in fact
    a game, with puzzles (such as a sequence that involves interactions
    between a rusty rod, an empty birdcage, a bird, and a snake), subtle
    humor (such as the surprising way that the bird helps the player get
    past the snake), and fantasy (including a magical crystal bridge, magic
    words, and combat with axe-wielding dwarves)."
    -- https://www.digitalhumanities.org/dhq/vol/001/2/000009/000009.html
       Accessed 21 May, 2024

    Original source files from Don Woods web site
        http://www.icynic.com/~don/jerz/advf4.77-03-31
        http://www.icynic.com/~don/jerz/advdat.77-03-31
    Accessed 5 May, 2024
*/

#include <algorithm>
#include <array>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>



/*
    ######## ########  ######  ########    ##       #### ########  
       ##    ##       ##    ##    ##       ##        ##  ##     ## 
       ##    ##       ##          ##       ##        ##  ##     ## 
       ##    ######    ######     ##       ##        ##  ########  
       ##    ##             ##    ##       ##        ##  ##     ## 
       ##    ##       ##    ##    ##       ##        ##  ##     ## 
       ##    ########  ######     ##       ######## #### ########  
*/

namespace micro_test_library {

/*  Define test functions with DEF_TEST_FUNC(function_name).
    Use TEST_EQUAL(value, expected_value) to test expected outcomes.
    Execute all test functions with RUN_TESTS(). */


// (used in test_equal() when called by test functions in this particular module)
template <typename T, size_t N>
std::ostream & operator<<(std::ostream & os, const std::array<T, N> & arr)
{
    os << '(';
    for (auto a : arr)
        os << a << ' ';
    os << ')';
    return os;
}

// (used in test_equal() when called by test functions in this particular module)
template <typename T>
std::ostream & operator<<(std::ostream & os, const std::vector<T> & vec)
{
    os << '(';
    for (auto v : vec)
        os << v << ' ';
    os << ')';
    return os;
}


unsigned test_count;      // total number of tests executed
unsigned fault_count;     // total number of tests that fail
std::vector<void (*)()> test_routines; // list of all test routines


// write a message to std::cout if !(value == expected_value)
template<typename A, typename B>
void test_equal(const A & value, const B & expected_value,
    const char * filename, const size_t line_num, const char * function_name)
{
    ++test_count;
    if (!(value == expected_value)) {
        ++fault_count;
        // e.g. love.cpp(2021) : in proposal() expected 'Yes!', but got 'no lol'
        std::cout
            << filename << '(' << line_num
            << ") : in " << function_name
            << "() expected '" << expected_value
            << "', but got '" << value
            << "'\n";
    }
}


// register a test function; return an arbitrary value
size_t add_test(void (*f)())
{
    test_routines.push_back(f);
    return test_routines.size();
}


// run all registered tests
void run_tests()
{
    for (auto & t : test_routines)
        t();
    if (fault_count)
        std::cout << fault_count << " total failures\n";
}


// write a message to std::cout if !(value == expected_value)
#define TEST_EQUAL(value, expected_value)                   \
{                                                           \
    micro_test_library::test_equal(value, expected_value,   \
        __FILE__, __LINE__, __FUNCTION__);                  \
}


// To allow test code to be placed nearby code being tested, test functions
// may be defined with this macro. All such functions may then be called
// with one call to RUN_TESTS(). Each test function must have a unique name.
#define DEF_TEST_FUNC(test_func)                                         \
void micro_test_##test_func();                                                        \
size_t micro_test_extern_##test_func = micro_test_library::add_test(micro_test_##test_func); \
void micro_test_##test_func()


// execute all the DEF_TEST_FUNC defined functions
#define RUN_TESTS() micro_test_library::run_tests()

} //namespace micro_test_library





/*
     ######   ######     ###    ######## ########  #######  ##       ########  
    ##    ## ##    ##   ## ##   ##       ##       ##     ## ##       ##     ## 
    ##       ##        ##   ##  ##       ##       ##     ## ##       ##     ## 
     ######  ##       ##     ## ######   ######   ##     ## ##       ##     ## 
          ## ##       ######### ##       ##       ##     ## ##       ##     ## 
    ##    ## ##    ## ##     ## ##       ##       ##     ## ##       ##     ## 
     ######   ######  ##     ## ##       ##        #######  ######## ########  
*/

namespace scaffolding {

// Supporting functions used to recreate some FORTRAN IV features


struct adventure_exception : public std::runtime_error {
    adventure_exception(const char * msg) : std::runtime_error(msg) {}
};

struct adventure_pause_exception : public adventure_exception {
    adventure_pause_exception() : adventure_exception("PAUSE: USER TERMINATED") {}
};



// return given string in uppercase
std::string to_upper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return static_cast<char>(std::toupper(c)); }
    );
    return s;
}


// five spaces in A5 format
constexpr uint_least64_t a5_space = 0201004020100ULL;


// Return given string in PDP-10 FORTRAN IV 36-bit integer format.
// String must contain between 0 and 5 ASCII characters.
uint_least64_t as_a5(const std::string & str)
{
    // Crowther wrote Adventure in FORTRAN IV for a DEC PDP-10. That machine
    // has 36-bit words. FORTRAN packs five 7-bit characters into one 36-bit
    // machine word, the least significant bit being unused. (Based on my
    // understanding of the masking and shifting in Crowther's GETIN
    // function.) The string is left justified, space padded to the right.

    if (str.size() > 5)
        throw std::out_of_range("as_a5(): given more than 5 characters");

    uint_least64_t result = 0;
    unsigned i = 0;
    while (i < str.size()) {
        result <<= 7;
        result |= str[i++] & 0x7F;
    }
    while (i++ < 5) {
        result <<= 7;
        result |= ' ';
    }
    result <<= 1;

    return result;
}

DEF_TEST_FUNC(as_a5)
{
    TEST_EQUAL(as_a5(""),       0201004020100ULL); // (= a5_space)
    TEST_EQUAL(as_a5("A"),      0405004020100ULL); // 'A' and 4 spaces
    TEST_EQUAL(as_a5("AB"),     0406044020100ULL);
    TEST_EQUAL(as_a5("ABC"),    0406050320100ULL);
    TEST_EQUAL(as_a5("ABCD"),   0406050342100ULL);
    TEST_EQUAL(as_a5("ABCDE"),  0406050342212ULL);
    TEST_EQUAL(as_a5(" BCDE"),  0202050342212ULL);
    TEST_EQUAL(as_a5("  CDE"),  0201010342212ULL);
    TEST_EQUAL(as_a5("   DE"),  0201004042212ULL);
    TEST_EQUAL(as_a5("    E"),  0201004020212ULL);
    TEST_EQUAL(as_a5("     "),  0201004020100ULL);
}


// Return given string as a vector of PDP-10 FORTRAN IV 36-bit integers.
std::vector<uint_least64_t> as_a5vec(const std::string & s)
{
    std::vector<uint_least64_t> result;
    const std::string buf{scaffolding::to_upper(s)};
    for (unsigned i = 0; i < buf.size(); i += 5)
        result.push_back(scaffolding::as_a5(buf.substr(i, 5)));
    return result;
}

DEF_TEST_FUNC(as_a5vec)
{
    std::vector<uint_least64_t> expected;
    expected = {};
    TEST_EQUAL(as_a5vec(""),                expected);
    expected = { 0405004020100ULL };
    TEST_EQUAL(as_a5vec("A"),               expected);
    expected = { 0406050342212ULL, 0406050342212ULL };
    TEST_EQUAL(as_a5vec("ABCDEABCDE"),      expected); 
    expected = { 0442131446236ULL, 0261012747644ULL, 0462104020100ULL };
    TEST_EQUAL(as_a5vec("hello, world"),    expected); 
}


// Return given A5 format integer as a string of 5 characters.
std::string as_string(uint_least64_t a)
{
    std::string result;
    a >>= 1;
    for (int i = 0; i < 5; ++i) {
        result.push_back((a >> ((4 - i) * 7)) & 0177);
    }
    return result;
}

DEF_TEST_FUNC(as_string)
{
    TEST_EQUAL(as_string(0201004020100ULL), "     ");
    TEST_EQUAL(as_string(0405004020100ULL), "A    ");
    TEST_EQUAL(as_string(0406044020100ULL), "AB   ");
    TEST_EQUAL(as_string(0406050320100ULL), "ABC  ");
    TEST_EQUAL(as_string(0406050342100ULL), "ABCD ");
    TEST_EQUAL(as_string(0406050342212ULL), "ABCDE");
    TEST_EQUAL(as_string(0202050342212ULL), " BCDE");
    TEST_EQUAL(as_string(0201010342212ULL), "  CDE");
    TEST_EQUAL(as_string(0201004042212ULL), "   DE");
    TEST_EQUAL(as_string(0201004020212ULL), "    E");
}


// The adventure function will communicate with the world, including
// the world of random numbers, through this interface.
class advent_io {
public:
    advent_io() = default;
    virtual ~advent_io() = default;

    // get the next line of text from the world
    virtual std::string getline() = 0;

    // output an ASCII string
    virtual void type(const std::string &) = 0;

    // output an integer
    virtual void type(int) = 0;

    // allows caller to record the current room id
    virtual void trace_location(int) {}

    // return a pseudo-random number between 0.0 and 1.0
    // (the int perameter is used only to trace the call location for testing)
    virtual double ran(int)
    {
        // I don't know what PRNG was used in Crowther's FORTRAN 4.
        return static_cast<double>(std::rand()) / RAND_MAX;
    }
};

std::string advent_io::getline() { return {}; }
void advent_io::type(const std::string &) {}
void advent_io::type(int) {}


// Output a string supplied in FORTRAN 4 (5 chars per 36-bit word) format.
void type_20a5(
    advent_io & io,
    const std::array<uint_least64_t, 23> & line,
    uint_least64_t begin,
    uint_least64_t end)
{
    std::string buf;
    for (uint_least64_t i = begin; i <= end; ++i)
        buf += scaffolding::as_string(line.at(i));
    buf += '\n';
    io.type(buf);
}


// Display the PAUSE text and wait for the user to type g or x.
void pause(advent_io & io, const char * msg)
{
    /*  "Execution of the PAUSE statement causes the message or the octal
         digits, if any, to be typed on the user's teletypewriter. Program
         execution may be resumed (at the next executable FORTRAN statement)
         from the console by typing "G," followed by a carriage return.
         Program execution may be terminated by typing "X," followed by
         carriage return."
         -- DEC-10-AFDO-D decsystem10 FORTRAN IV Programmer's Reference Manual
            dated 1967-1972

        I don't know exactly what the user would see. Just the PAUSE text?
        The SunSoft FORTRAN 77 4.0 Reference Manual gives this example:

            PAUSE: 1
            To resume execution, type: go
            Any other input will terminate the program.
            go
            Execution resumed after PAUSE.

        I've combined these two into the following:
    */
    io.type("PAUSE: ");
    io.type(msg);
    io.type("\n");

    for (;;) {
        io.type("TO RESUME EXECUTION, TYPE: G\n"
                "TO TERMINATE THE PROGRAM, TYPE: X\n");
        const auto input{to_upper(io.getline())};
        if (input == "G") {
            io.type("EXECUTION RESUMED\n\n");
            break;
        }
        if (input == "X")
            throw adventure_pause_exception();
    }
};


// "The ACCEPT statement reads from standard input." -- FORTRAN IV
// In Adventure, ACCEPT is used in only one place with a format specifier
// of "4A5", i.e. 4 ints each holding 5 characters.
void accept_4A5(advent_io & io, std::array<uint_least64_t, 6> & a)
{
    std::vector<uint_least64_t> line(as_a5vec(io.getline()));

    a[0] = 9999; // (a[0] is unused)
    for (unsigned i = 0;  i < 4; ++i)
        a[i + 1] = (i < line.size()) ? line[i] : a5_space;

    // There is one place in the original code where ACCEPT is called:
    //      6       ACCEPT 1,(A(I), I=1,4)
    //      1       FORMAT(4A5)
    // But there A is dimensioned as A(5) and the last element is later
    // referenced. How is A(5) initialised in that case? Here we assume
    // it's space filled.
    a[5] = a5_space;
};

}// namespace scaffolding





/*
     ######  ########   #######  ##      ## ######## ##     ## ######## ########  
    ##    ## ##     ## ##     ## ##  ##  ##    ##    ##     ## ##       ##     ## 
    ##       ##     ## ##     ## ##  ##  ##    ##    ##     ## ##       ##     ## 
    ##       ########  ##     ## ##  ##  ##    ##    ######### ######   ########  
    ##       ##   ##   ##     ## ##  ##  ##    ##    ##     ## ##       ##   ##   
    ##    ## ##    ##  ##     ## ##  ##  ##    ##    ##     ## ##       ##    ##  
     ######  ##     ##  #######   ###  ###     ##    ##     ## ######## ##     ## 
*/

namespace Crowther {

// Original Adventure subroutines
// (These appear after the Adventure END statement in the original code.)

                                                    //         SUBROUTINE SHIFT (VAL,DIST,RES)
void shift(uint_least64_t val, int dist, uint_least64_t & res)
{
                                                    //         IMPLICIT INTEGER (A-Z)
                                                    //         RES=VAL
                                                    //         IF(DIST)10,20,30
                                                    // 10      IDIST=-DIST
                                                    //         DO 11 I=1,IDIST
                                                    //         J = 0
                                                    //         IF (RES.LT.0) J="200000000000
                                                    // 11      RES = ((RES.AND."377777777777)/2) + J
                                                    // 20      RETURN
                                                    // 30      DO 31 I=1,DIST
                                                    //         j = 0
                                                    //         IF ((RES.AND."200000000000).NE.0) J="400000000000
                                                    // 31      RES = (RES.AND."177777777777)*2 + J
                                                    //         RETURN
                                                    //         END

    // [Wikipedia says the PDP-10 does two's complement 36-bit integer arithmetic.
    //  The above code appears to be designed to shift a 36-bit unsigned integer
    //  using signed arithmetic. The code has to test whether the top-bit is set,
    //  mask it out then add it back in after shifting left or right. We are using
    // 36 bits of a 64-bit unsigned number so the below code is simpler.]
    res = (dist < 0) ? val >> -dist : (val << dist) & 0777777777777ULL;
}

DEF_TEST_FUNC(shift)
{
    uint_least64_t result = 0;
    shift(0000000000001ULL,   1, result);
    TEST_EQUAL(result, 0000000000002ULL);
    shift(0000000000001ULL,  -1, result);
    TEST_EQUAL(result, 0000000000000ULL);
    shift(0000000000001ULL,  35, result);
    TEST_EQUAL(result, 0400000000000ULL);
    shift(0400000000000ULL, -35, result);
    TEST_EQUAL(result, 0000000000001ULL);
    shift(0444444444444ULL,  18, result);
    TEST_EQUAL(result, 0444444000000ULL);
    shift(0444444444444ULL, -18, result);
    TEST_EQUAL(result, 0000000444444ULL);
    shift(0123456701234ULL,   0, result);
    TEST_EQUAL(result, 0123456701234ULL);
}


                                                    //         SUBROUTINE GETIN(TWOW,B,C,D)
void getin(
        scaffolding::advent_io & io,
        uint_least64_t & twow,  // [0 means one word was input; 1 means at least two words]
        uint_least64_t & b,     // [up to five characters of the first word]
        uint_least64_t & c,     // [up to five characters of the second word (when twow == 1)]
        uint_least64_t & d)     // [characters 6..10 of the user's input (or spaces)]
{
    int s;                                          //         IMPLICIT INTEGER(A-Z)
    std::array<uint_least64_t, 6> a{};              //         DIMENSION A(5),M2(6)
    std::array<uint_least64_t, 7> m2 {              //         DATA M2/"4000000000,"20000000,"100000,"400,"2,0/
        9999ULL,04000000000ULL,020000000ULL,0100000ULL,0400ULL,02ULL,0ULL
    };
    uint_least64_t xx, yy, mask;
    scaffolding::accept_4A5(io, a);                 // 6       ACCEPT 1,(A(I), I=1,4)
    // [Only the first 4 elements of A are initialised?
    // Yet A(5) is referenced below when J=4, SHIFT(A(J+1),...?]
                                                    // 1       FORMAT(4A5)
    twow = 0;                                       //         TWOW=0
    s = 0;                                          //         S=0
    // [S=0: no space character found yet; S=1: space found]
    b = a[1];                                       //         B=A(1)
    for (int j = 1; j <= 4;++j) {                   //         DO 2 J=1,4
        for (int k = 1; k <= 5; ++k) {              //         DO 2 K=1,5
            uint_least64_t mask1=0774000000000ULL;  //         MASK1="774000000000
            if (k != 1) mask1 = 0177ULL * m2[k];    //         IF(K.NE.1) MASK1="177*M2(K)

            // [Note that 0201004020100 is 5 spaces in A5 format, so the following line will jump
            // to L3 if character K (character 1 is the leftmost) in word A[J] is a space.]
                                                    //         IF(((A(J).XOR."201004020100).AND.MASK1).EQ.0)GOTO 3
            if (((a[j] ^ 0201004020100ULL) & mask1) == 0) goto L3;

            // [So character (J,K) is not a space. If we have never yet seen a space
            // just continue looking.]
            if (s == 0) continue;                   //         IF(S.EQ.0) GOTO 2

            // [Encountered a non-space character following one or more spaces (S is non-zero).
            // Therefore there are at least two words in the response, hence TWOW set to 1.
            // (But that would only be true if there could not be leading spaces in the input?)]
            twow = 1;                               //         TWOW=1

            // [Shift the start of the second word into the top of XX and what follows the
            // start of the second word into the bottom of YY, then mask and join XX an YY
            // to form the properly aligned second word, which is recorded in C.
            // (Note that the second "word" is better described as any 5 characters beginning
            // with a non-space character following the first word. E.g. if the input is
            // "WHO ARE YOU", GETIN will return TWOW=1, B="WHO  ", C="ARE Y", D="RE YO".
            // However, the user is instructed to "DIRECT ME WITH COMMANDS OF 1 OR 2 WORDS"
            // so this should never be a problem...)]
            shift(a.at(j    ), 7*(k-1), xx);        //         CALL SHIFT(A(J),7*(K-1),XX)
            shift(a.at(j + 1), 7*(k-6), yy);        //         CALL SHIFT(A(J+1),7*(K-6),YY)
            mask = 0-m2[6 - k];                     //         MASK=-M2(6-K)
            c = (xx & mask) + (yy & (-2-mask));     //         C=(XX.AND.MASK)+(YY.AND.(-2-MASK))
            goto L4;                                //         GOTO 4

            // [If this isn't the first space we've seen just continue looking for the
            // start of a second word.]
L3:         if (s == 1) continue;                   // 3       IF(S.EQ.1) GOTO 2

            // [We've found the first space following the first word.]
            s = 1;                                  //         S=1

            // [If the first word fits wholly in A(1), replace any characters following
            // it with spaces so that B is just the first word, possibly with trailing spaces.]
            if (j == 1) b = (b & 0-m2[k]) |          //         IF(J.EQ.1) B=(B.AND.-M2(K)).OR.("201004020100.AND.
                (0201004020100ULL & (0-m2[k] ^ -1)); //       1 (-M2(K).XOR.-1))
        }
    }                                               // 2       CONTINUE
L4: d = a[2];                                       // 4       D=A(2)
                                                    //         RETURN
};                                                  //         END

DEF_TEST_FUNC(getin)
{
    class advent_io_getin_test : public scaffolding::advent_io {
        std::string line_;
    public:
        explicit advent_io_getin_test(const std::string & line) : line_(line) {}
        virtual ~advent_io_getin_test() {}
        std::string getline() override { return line_; }
        void type(const std::string & msg) override {}
        void type(int n) override {}
    };

    {
        advent_io_getin_test io("xyzzy");
        uint_least64_t twow{99}, b{99}, c{99}, d{99};
        getin(io, twow, b, c, d);
        TEST_EQUAL(twow, 0);
        TEST_EQUAL(b, scaffolding::as_a5("XYZZY"));
        TEST_EQUAL(c, 99);
        TEST_EQUAL(d, scaffolding::as_a5("     "));
    }
    {
        advent_io_getin_test io("Supercalifragilisticexpialidocious          ");
        uint_least64_t twow{99}, b{99}, c{99}, d{99};
        getin(io, twow, b, c, d);
        TEST_EQUAL(twow, 0);
        TEST_EQUAL(b, scaffolding::as_a5("SUPER"));
        TEST_EQUAL(c, 99);
        TEST_EQUAL(d, scaffolding::as_a5("CALIF"));
    }
    {
        advent_io_getin_test io("go           west");
        uint_least64_t twow{99}, b{99}, c{99}, d{99};
        getin(io, twow, b, c, d);
        TEST_EQUAL(twow, 1);
        TEST_EQUAL(b, scaffolding::as_a5("GO   "));
        TEST_EQUAL(c, scaffolding::as_a5("WEST "));
        TEST_EQUAL(d, scaffolding::as_a5("     "));
    }
    {
        advent_io_getin_test io("WHO ARE YOU");
        uint_least64_t twow{99}, b{99}, c{99}, d{99};
        getin(io, twow, b, c, d);
        TEST_EQUAL(twow, 1);
        TEST_EQUAL(b, scaffolding::as_a5("WHO  "));
        TEST_EQUAL(c, scaffolding::as_a5("ARE Y"));
        TEST_EQUAL(d, scaffolding::as_a5("RE YO"));
    }
}


                                                    //         SUBROUTINE SPEAK(IT)
void speak(
    scaffolding::advent_io & io,
    const std::array<int, 101> & rtext,
    const std::vector<std::array<uint_least64_t, 23>> & lline,
    int it)
{
    auto typ = [&](
            const std::array<uint_least64_t, 23> & line,
            uint_least64_t begin,
            uint_least64_t end) {
        scaffolding::type_20a5(io, line, begin, end);
    };

    int kkt;                                        //         IMPLICIT INTEGER(A-Z)
                                                    //         COMMON RTEXT,LLINE
                                                    //         DIMENSION RTEXT(100),LLINE(1000,22)

    kkt = rtext.at(it);                             //         KKT=RTEXT(IT)
    if (kkt == 0) return;                           //         IF(KKT.EQ.0)RETURN
L999: typ(lline.at(kkt), 3, lline.at(kkt)[2]);      // 999     TYPE 998, (LLINE(KKT,JJT),JJT=3,LLINE(KKT,2))
                                                    // 998     FORMAT(20A5)
    ++kkt;                                          //         KKT=KKT+1
    if (lline.at(kkt-1)[1] != 0) goto L999;         //         IF(LLINE(KKT-1,1).NE.0)GOTO 999
                                                    // 997     TYPE 996
    io.type("\n");                                  // 996     FORMAT(/)
                                                    //         RETURN
};                                                  //         END




                                                    //         SUBROUTINE YES(X,Y,Z,YEA)
void yes(
    scaffolding::advent_io & io,
    const std::array<int, 101> & rtext,
    const std::vector<std::array<uint_least64_t, 23>> & lline,
    int x,          // [index of text of question to be asked]
    int y,          // [index of text of response if user doesn't say no]
    int z,          // [index of text of response if user says no]
    int & yea)      // [0: user said no; 1: user didn't say no]
{
    auto speak = [&](int it) { Crowther::speak(io, rtext, lline, it); };

    uint_least64_t junk, ia1, ib1;                  //         IMPLICIT INTEGER(A-Z)
    speak(x);                                       //         CALL SPEAK(X)
    getin(io, junk, ia1, junk, ib1);                //         CALL GETIN(JUNK,IA1,JUNK,IB1)
                                                    //         IF(IA1.EQ.'NO'.OR.IA1.EQ.'N') GOTO 1
    if (ia1 == scaffolding::as_a5("NO") || ia1== scaffolding::as_a5("N")) goto L1;
    yea = 1;                                        //         YEA=1
    if (y != 0) speak(y);                           //         IF(Y.NE.0) CALL SPEAK(Y)
    return;                                         //         RETURN
L1: yea = 0;                                        // 1       YEA=0
    if (z != 0) speak(z);                           //         IF(Z.NE.0)CALL SPEAK(Z)
                                                    //         RETURN
};                                                  //         END





/*
       ###    ########  ##     ## ######## ##    ## ######## ##     ## ########  ######## 
      ## ##   ##     ## ##     ## ##       ###   ##    ##    ##     ## ##     ## ##       
     ##   ##  ##     ## ##     ## ##       ####  ##    ##    ##     ## ##     ## ##       
    ##     ## ##     ## ##     ## ######   ## ## ##    ##    ##     ## ########  ######   
    ######### ##     ##  ##   ##  ##       ##  ####    ##    ##     ## ##   ##   ##       
    ##     ## ##     ##   ## ##   ##       ##   ###    ##    ##     ## ##    ##  ##       
    ##     ## ########     ###    ######## ##    ##    ##     #######  ##     ## ########
*/

// Adventure -- recoded in C++ as directly as seemed reasonable
// (Original source: http://www.icynic.com/~don/jerz/advf4.77-03-31)
template <typename input_stream>
void adventure(
    input_stream & advdat,          // Adventure data file stream
    scaffolding::advent_io & io)    // communication with outside world
{
    using excpt = scaffolding::adventure_exception;
    auto pause = [&](const char * msg) { scaffolding::pause(io, msg); };


    // read a line from the map data table to mimic FORMAT(12G), as in
    //      1014    READ(1,1015)JKIND,LKIND,(TK(L),L=1,10)
    //      1015    FORMAT(12G)
    auto rdmap = [&](int & jkind, int & lkind, std::array<int, 26> & tk) {
        if (!(advdat >> jkind))
            throw excpt("rdmap(): read jkind failed");
        std::string buf;
        std::getline(advdat, buf);
        std::istringstream iss(buf);
        if (!(iss >> lkind))
            lkind = 0;
        int i = 1;
        while (i <= 10 && (iss >> tk[i]))
            ++i;
        while (i <= 10)
            tk[i++] = 0;
    };

    // read a line from the text table to mimic FORMAT(1G,20A5), as in
    //      1004    READ(1,1005)JKIND,(LLINE(I,J),J=3,22)
    //      1005    FORMAT(1G,20A5)
    auto rdtext = [&](int & j, std::array<uint_least64_t, 23> & t) {
        if (!(advdat >> j))
            throw excpt("rdtext(): read jkind failed");
        std::string buf;
        std::getline(advdat, buf);
        while (!buf.empty() && buf[0] == ' ')
            buf.erase(0, 1);
        t[0] = 9999;
        t[1] = 0;
        t[2] = 0;
        int n = 3;
        while (n < 23 && !buf.empty()) {
            t[n++] = scaffolding::as_a5(buf.substr(0, 5));
            buf.erase(0, 5);
        }
        while (n < 23)
            t[n++] = scaffolding::as_a5("");
    };

    // read a line from the keyword table to mimic FORMAT(G,A5), as in
    //              READ(1,1021) KTAB(IU),ATAB(IU)
    //      1021    FORMAT(G,A5)
    auto rdkey = [&](int & k, uint_least64_t & a) {
        if (!(advdat >> k))
            throw excpt("rdkey(): read k failed");
        std::string buf;
        std::getline(advdat, buf);
        while (!buf.empty() && buf[0] == ' ')
            buf.erase(0, 1);
        a = scaffolding::as_a5(buf.substr(0, 5));
    };


                                                    //       C ADVENTURES
                                                    //         IMPLICIT INTEGER(A-Z)
    int attack, dtot, id, idark, idetal, idwarf,
        ifirst, ikind, iid, il, ilk, ilong, itemp, iwest;
    int j, jkind, jobj, jspk(9999), jverb(9999), k(9999),
        kk, kq, ktem, l, lkind, ll, loc, lold(9999), ltrubl,
        stick, temp, yea;
    uint_least64_t a, b, twowds, wd2;

                                                    //         REAL RAN
                                                    //         COMMON RTEXT,LLINE
                                                    //         DIMENSION IOBJ(300),ICHAIN(100),IPLACE(100)
                                                    //       1 ,IFIXED(100),COND(300),PROP(100),ABB(300),LLINE(1000,22)
                                                    //       2 ,LTEXT(300),STEXT(300),KEY(300),DEFAULT(300),TRAVEL(1000)
                                                    //       3 ,TK(25),KTAB(1000),ATAB(1000),BTEXT(200),DSEEN(10)
                                                    //       4 ,DLOC(10),ODLOC(10),DTRAV(20),RTEXT(100),JSPKT(100)
                                                    //       5 ,IPLT(100),IFIXT(100)
    std::array<int, 11>   dloc{},dseen{},odloc{};
    std::array<int, 26>   tk{};
    std::array<int, 101>  ichain{},ifixed{},iplace{},prop{},rtext{};
    std::array<int, 201>  btext{};
    std::array<int, 301>  abb{},cond{},default_{},iobj{},key{},ltext{},stext{};
    std::vector<int> ktab(1001), travel(1001);
    std::vector<std::array<uint_least64_t, 23>> lline(1001); // [description text table]
    std::vector<uint_least64_t> atab(1001); // [keyword table, each keyword is 1..5 7-bit characters]

    auto speak = [&](int it) { Crowther::speak(io, rtext, lline, it); };
    auto yes = [&](int x, int y, int z, int & yea) { Crowther::yes(io, rtext, lline, x, y, z, yea); };




                                                    //       C READ THE PARAMETERS

                                                    //         IF(SETUP.NE.0) GOTO 1
                                                    //         SETUP=1
    // [SETUP is not explicitly initialised prior to the IF. Presumably the PDP-10
    // FORTRAN system could be relied upon to zero-initialise variables? Anyway,
    // how could execution return to this point to ask whether SETUP was non-zero?]

    constexpr int keys      = 1;                    //         KEYS=1
    constexpr int lamp      = 2;                    //         LAMP=2
    constexpr int grate     = 3;                    //         GRATE=3
    //           [cage      = 4]
    constexpr int rod       = 5;                    //         ROD=5
    //           [steps     = 6]
    constexpr int bird      = 7;                    //         BIRD=7
    constexpr int nugget    = 10;                   //         NUGGET=10
    constexpr int snake     = 11;                   //         SNAKE=11
    //           [fissure   = 12]
    //           [diamonds  = 13]
    //           [silver    = 14]
    //           [jewels    = 15]
    //           [coins     = 16]
    //           [dwarves   = 17]
    //           [knife/rock = 18]
    constexpr int food      = 19;                   //         FOOD=19
    constexpr int water     = 20;                   //         WATER=20
    constexpr int axe       = 21;                   //         AXE=21
    //           [knife     = 22]
    //           [chest     = 23]
                                                    //         DATA(JSPKT(I),I=1,16)/24,29,0,31,0,31,38,38,42,42,43,46,77,71
                                                    //       1 ,73,75/
    std::array<int, 101> jspkt = {
        9999,24,29,0,31,0,31,38,38,42,42,43,46,77,71,73,75
    };
                                                    //         DATA(IPLT(I),I=1,20)/3,3,8,10,11,14,13,9,15,18,19,17,27,28,29
                                                    //       1 ,30,0,0,3,3/
    // [Initial location of objects. E.g. iplt[keys] = room 3, i.e. in the building]
    std::array<int, 101> iplt = {
        9999,3,3,8,10,11,14,13,9,15,18,19,17,27,28,29,30,0,0,3,3
    };
                                                    //         DATA(IFIXT(I),I=1,20)/0,0,1,0,0,1,0,1,1,0,1,1,0,0,0,0,0,0,0,0/
    std::array<int, 101> ifixt = {
        9999,0,0,1,0,0,1,0,1,1,0,1,1
    };
                                                    //         DATA(DTRAV(I),I=1,15)/36,28,19,30,62,60,41,27,17,15,19,28,36
                                                    //       1 ,300,300/
    std::array<int, 21> dtrav = {
        9999,36,28,19,30,62,60,41,27,17,15,19,28,36,300,300
    };
                                                    //         DO 1001 I=1,300
                                                    //         STEXT(I)=0
                                                    //         IF(I.LE.200) BTEXT(I)=0
                                                    //         IF(I.LE.100)RTEXT(I)=0
                                                    // 1001    LTEXT(I)=0
    int i = 1;                                      //         I=1
                                                    //         CALL IFILE(1,'TEXT')
L1002: if (!(advdat >> ikind))                      // 1002    READ(1,1003) IKIND
        throw excpt("L1002: read ikind failed");    // 1003    FORMAT(G)
                                                    //         GOTO(1100,1004,1004,1013,1020,1004,1004)(IKIND+1)
    switch (ikind) {
    case 0: goto L1100; // [end of data]
    case 1: goto L1004; // [long descriptions]
    case 2: goto L1004; // [short descriptions]
    case 3: goto L1013; // [map data]
    case 4: goto L1020; // [keywords]
    case 5: goto L1004; // [game state descriptions]
    case 6: goto L1004; // [events]
    default: throw excpt("L1002: unexpected ikind value");
    }

L1004: rdtext(jkind, lline[i]);                     // 1004    READ(1,1005)JKIND,(LLINE(I,J),J=3,22)
                                                    // 1005    FORMAT(1G,20A5)
    if (jkind == -1) goto L1002;                    //         IF(JKIND.EQ.-1) GOTO 1002
    for (k = 1; k <= 20; ++k) {                     //         DO 1006 K=1,20
        kk = k;                                     //         KK=K
        // [Is the following a bug? 1004 reads up to 20 5-character values into linei[3]..linei[22].
        // This loop looks backwards for the first non-blank 5-character value from linei[20]..linei[1];
        // line[1] & line[2] do not contain text and linei[21] & linei[22] are not tested. The bug
        // would not have manifested itself with advdat.77-03-31 as no line in that text is longer
        // than linei[17] and no line is blank.]
                                                    //         IF(LLINE(I,21-K).NE.' ') GOTO 1007
        if (lline.at(i)[21-k] != scaffolding::a5_space) goto L1007;
    }                                               // 1006    CONTINUE
    throw excpt("L1004: unexpected blank line");    //         STOP
L1007: lline[i][2] = 20 - kk + 1;                   // 1007    LLINE(I,2)=20-KK+1
    lline[i][1] = 0;                                //         LLINE(I,1)=0
    if (ikind == 6) goto L1023;                     //         IF(IKIND.EQ.6)GOTO 1023
    if (ikind == 5) goto L1011;                     //         IF(IKIND.EQ.5)GOTO 1011
    if (ikind == 1) goto L1008;                     //         IF(IKIND.EQ.1) GOTO 1008
    if (stext.at(jkind) != 0) goto L1009;           //         IF(STEXT(JKIND).NE.0) GOTO 1009
    stext[jkind] = i;                               //         STEXT(JKIND)=I
    goto L1010;                                     //         GOTO 1010
                                                    //
L1008: if (ltext.at(jkind) != 0) goto L1009;        // 1008    IF(LTEXT(JKIND).NE.0) GOTO 1009
    ltext[jkind] = i;                               //         LTEXT(JKIND)=I
    goto L1010;                                     //         GOTO 1010
L1009: lline.at(i - 1)[1]= i;                       // 1009    LLINE(I-1,1)=I
L1010: ++i;                                         // 1010    I=I+1
    if (i != 1000) goto L1004;                      //         IF(I.NE.1000)GOTO 1004
    pause("TOO MANY LINES");                        //         PAUSE 'TOO MANY LINES'
    // [If pause returns (i.e. the user chose to continue) we will fall into the code below.
    // That doesn't look logical here. I assume using PAUSE was a convenient way
    // to stop the execution - with a message - so you know why it stopped.]
                                                    //
L1011: if (jkind < 200) goto L1012;                 // 1011    IF(JKIND.LT.200)GOTO 1012
    if (btext.at(jkind - 100) != 0) goto L1009;     //         IF(BTEXT(JKIND-100).NE.0)GOTO 1009
    btext[jkind - 100] = i;                         //         BTEXT(JKIND-100)=I
    btext[jkind - 200] = i;                         //         BTEXT(JKIND-200)=I
    goto L1010;                                     //         GOTO 1010
L1012: if (btext.at(jkind) != 0) goto L1009;        // 1012    IF(BTEXT(JKIND).NE.0)GOTO 1009
    btext[jkind] = i;                               //         BTEXT(JKIND)=I
    goto L1010;                                     //         GOTO 1010
                                                    //
L1023: if (rtext.at(jkind) != 0) goto L1009;        // 1023    IF(RTEXT(JKIND).NE.0) GOTO 1009
    rtext[jkind] = i;                               //         RTEXT(JKIND)=I
    goto L1010;                                     //         GOTO 1010
                                                    //
L1013: i = 1;                                       // 1013    I=1
L1014: rdmap(jkind, lkind, tk);                     // 1014    READ(1,1015)JKIND,LKIND,(TK(L),L=1,10)
                                                    // 1015    FORMAT(12G)
    if (jkind == -1) goto L1002;                    //         IF(JKIND.EQ.-1) GOTO 1002
    if (key.at(jkind) != 0) goto L1016;             //         IF(KEY(JKIND).NE.0) GOTO 1016
    key[jkind] = i;                                 //         KEY(JKIND)=I
    goto L1017;                                     //         GOTO 1017
L1016: travel.at(i - 1) = -travel.at(i - 1);        // 1016    TRAVEL(I-1)=-TRAVEL(I-1)
L1017: for (l = 1; l <= 10; ++l) {                  // 1017    DO 1018 L=1,10
        if (tk[l] == 0) goto L1019;                 //         IF(TK(L).EQ.0) GOTO 1019
        travel.at(i) = lkind * 1024 + tk[l];        //         TRAVEL(I)=LKIND*1024+TK(L)
        ++i;                                        //         I=I+1
        if (i == 1000) throw excpt("L1017: STOP");  //         IF(I.EQ.1000) STOP
    }                                               // 1018    CONTINUE
L1019: travel.at(i - 1) = -travel.at(i - 1);        // 1019    TRAVEL(I-1)=-TRAVEL(I-1)
    goto L1014;                                     //         GOTO 1014
                                                    //
L1020: for (int iu = 1; iu <= 1000; ++iu) {         // 1020    DO 1022 IU=1,1000
        rdkey(ktab[iu], atab[iu]);                  //         READ(1,1021) KTAB(IU),ATAB(IU)
                                                    // 1021    FORMAT(G,A5)
        if (ktab[iu] == -1) goto L1002;             //         IF(KTAB(IU).EQ.-1)GOTO 1002
    }                                               // 1022    CONTINUE
    pause("TOO MANY WORDS");                        //         PAUSE 'TOO MANY WORDS'
    // [Falling into the code below doesn't look logical. See my comment above.]
                                                    //
                                                    //
                                                    //       C TRAVEL = NEG IF LAST THIS SOURCE + DEST*1024 + KEYWORD
                                                    //
                                                    //       C COND  = 1 IF LIGHT,  2 IF DON'T ASK QUESTION
                                                    // 
                                                    // 
                                                    // 
                                                    // 
                                                    // 
L1100: for (i = 1; i <= 100; ++i) {                 // 1100    DO 1101 I=1,100
        iplace[i] = iplt[i];                        //         IPLACE(I)=IPLT(I)
        ifixed[i] = ifixt[i];                       //         IFIXED(I)=IFIXT(I)
        // [ichain was zero initialised]            // 1101    ICHAIN(I)=0
    }                                               //
                                                    //         DO 1102 I=1,300
    // [abb, cond & iobj were zero initialised]     //         COND(I)=0
                                                    //         ABB(I)=0
                                                    // 1102    IOBJ(I)=0
    for (i = 1; i <= 10; ++i)                       //         DO 1103 I=1,10
        cond[i] = 1; // [locs 1..10 have light]     // 1103    COND(I)=1
    cond[16] = 2;                                   //         COND(16)=2
    cond[20] = 2;                                   //         COND(20)=2
    cond[21] = 2;                                   //         COND(21)=2
    cond[22] = 2;                                   //         COND(22)=2
    cond[23] = 2;                                   //         COND(23)=2
    cond[24] = 2;                                   //         COND(24)=2
    cond[25] = 2;                                   //         COND(25)=2
    cond[26] = 2;                                   //         COND(26)=2
    cond[31] = 2;                                   //         COND(31)=2
    cond[32] = 2;                                   //         COND(32)=2
    cond[79] = 2;                                   //         COND(79)=2
                                                    //
    for (i = 1; i <= 100; ++i) {                    //         DO 1107 I=1,100
        ktem = iplace[i];                           //         KTEM=IPLACE(I)
        if (ktem == 0) continue;                    //         IF(KTEM.EQ.0)GOTO 1107
        if (iobj.at(ktem) != 0) goto L1104;         //         IF(IOBJ(KTEM).NE.0) GOTO 1104
        iobj.at(ktem) = i;                          //         IOBJ(KTEM)=I
        continue;                                   //         GO TO 1107
L1104:  ktem = iobj.at(ktem);                       // 1104    KTEM=IOBJ(KTEM)
L1105:  if (ichain.at(ktem) != 0) goto L1106;       // 1105    IF(ICHAIN(KTEM).NE.0) GOTO 1106
        ichain.at(ktem) = i;                        //         ICHAIN(KTEM)=I
        continue;                                   //         GOTO 1107
L1106:  ktem = ichain.at(ktem);                     // 1106    KTEM=ICHAIN(KTEM)
        goto L1105;                                 //         GOTO 1105
    }                                               // 1107    CONTINUE
    idwarf = 0;                                     //         IDWARF=0
    ifirst = 1;                                     //         IFIRST=1
    iwest = 0;                                      //         IWEST=0
    ilong = 1;                                      //         ILONG=1
    idetal = 0;                                     //         IDETAL=0
    pause("INIT DONE");                             //         PAUSE 'INIT DONE'
                                                    // 
                                                    // 
                                                    // 
    // [65:"WELCOME TO ADVENTURE!!  WOULD YOU LIKE INSTRUCTIONS?"]
    yes(65, 1, 0, yea);                             // 1       CALL YES(65,1,0,YEA)
    l = 1;                                          //         L=1
    loc = 1;                                        //         LOC=1
L2:
    io.trace_location(l); // [This line is not part of Crowther's code.]

    // [The following line is not in Crowther's code. It was added to avoid
    //  an infinite loop. See note at L25.]
    if (l == 26) pause("GAME OVER");

    for (i = 1; i <= 3; ++i) {                      // 2       DO 73 I=1,3
        if (odloc[i]!=l || dseen[i]==0) continue;   //         IF(ODLOC(I).NE.L.OR.DSEEN(I).EQ.0)GOTO 73
        l = loc;                                    //         L=LOC
        // [2:"A LITTLE DWARF WITH A BIG KNIFE BLOCKS YOUR WAY."]
        speak(2);                                   //         CALL SPEAK(2)
        goto L74;                                   //         GOTO 74
    }                                               // 73      CONTINUE
L74:loc = l;                                        // 74      LOC=L
                                                    //
                                                    //       C DWARF STUFF
                                                    //
    if (idwarf != 0) goto L60;                      //         IF(IDWARF.NE.0) GOTO 60
    if (loc == 15) idwarf = 1;                      //         IF(LOC.EQ.15) IDWARF=1
    goto L71;                                       //         GOTO 71
L60:if (idwarf != 1) goto L63;                      // 60      IF(IDWARF.NE.1)GOTO 63
    if (io.ran(60) > 0.05) goto L71;                //         IF(RAN(QZ).GT.0.05) GOTO 71
    idwarf = 2;                                     //         IDWARF=2
    for (i = 1; i <= 3; ++i) {                      //         DO 61 I=1,3
        dloc[i] = 0;                                //         DLOC(I)=0
        odloc[i] = 0;                               //         ODLOC(I)=0
        dseen[i] = 0;                               // 61      DSEEN(I)=0
    }
    // [3:"A LITTLE DWARF...THREW A LITTLE AXE AT YOU WHICH MISSED..."]
    speak(3);                                       //         CALL SPEAK(3)
    ichain.at(axe) = iobj.at(loc);                  //         ICHAIN(AXE)=IOBJ(LOC)
    iobj[loc] = axe;                                //         IOBJ(LOC)=AXE
    iplace[axe] = loc;                              //         IPLACE(AXE)=LOC
    goto L71;                                       //         GOTO 71
                                                    //
L63:++idwarf;                                       // 63      IDWARF=IDWARF+1
    attack = 0;                                     //         ATTACK=0
    dtot = 0;                                       //         DTOT=0
    stick = 0;                                      //         STICK=0
    for (i = 1; i <=3; ++i) {                       //         DO 66 I=1,3
        if (2 * i + idwarf < 8) continue;           //         IF(2*I+IDWARF.LT.8)GOTO 66
                                                    //         IF(2*I+IDWARF.GT.23.AND.DSEEN(I).EQ.0)GOTO 66
        if (2 * i + idwarf > 23 && dseen[i] == 0) continue;
        odloc[i] = dloc[i];                         //         ODLOC(I)=DLOC(I)
        if (dseen[i] != 0 && loc > 14) goto L65;    //         IF(DSEEN(I).NE.0.AND.LOC.GT.14)GOTO 65
        dloc[i] = dtrav.at(i * 2 + idwarf - 8);     //         DLOC(I)=DTRAV(I*2+IDWARF-8)
        dseen[i] = 0;                               //         DSEEN(I)=0
        if (dloc[i]!=loc && odloc[i]!=loc) continue;//         IF(DLOC(I).NE.LOC.AND.ODLOC(I).NE.LOC) GOTO 66
L65:    dseen[i] = 1;                               // 65      DSEEN(I)=1
        dloc[i] = loc;                              //         DLOC(I)=LOC
        ++dtot;                                     //         DTOT=DTOT+1
        if (odloc[i] != dloc[i]) continue;          //         IF(ODLOC(I).NE.DLOC(I)) GOTO 66
        ++attack;                                   //         ATTACK=ATTACK+1
        if (io.ran(65) < 0.1) ++stick;              //         IF(RAN(QZ).LT.0.1) STICK=STICK+1
    }                                               // 66      CONTINUE
    if (dtot == 0) goto L71;                        //         IF(DTOT.EQ.0) GOTO 71
    if (dtot == 1) goto L75;                        //         IF(DTOT.EQ.1)GOTO 75
    io.type("THERE ARE "); io.type(dtot);           //         TYPE 67,DTOT
        io.type(" THREATENING LITTLE DWARVES IN"    // 67      FORMAT(' THERE ARE ',I2,' THREATENING LITTLE DWARVES IN THE
                 " THE ROOM WITH YOU.\n");          //       1  ROOM WITH YOU.',/)
    goto L77;                                       //         GOTO 77
    // [4:"THERE IS A THREATENING LITTLE DWARF IN THE ROOM WITH YOU!"]
L75:speak(4);                                       // 75      CALL SPEAK(4)
L77:if (attack == 0) goto L71;                      // 77      IF(ATTACK.EQ.0)GOTO 71
    if (attack == 1) goto L79;                      //         IF(ATTACK.EQ.1)GOTO 79
    io.type(" "); io.type(attack);                  //         TYPE 78,ATTACK
    io.type(" OF THEM THROW KNIVES AT YOU!\n");     // 78      FORMAT(' ',I2,' OF THEM THROW KNIVES AT YOU!',/)
    goto L81;                                       //         GOTO 81
    // [5:"ONE SHARP NASTY KNIFE IS THROWN AT YOU!"]
L79:speak(5);                                       // 79      CALL SPEAK(5)
    // [52:"IT MISSES!" 53:"IT GETS YOU!"]
    speak(52 + stick);                              //         CALL SPEAK(52+STICK)
    switch (stick + 1) {                            //         GOTO(71,83)(STICK+1)
        case 1: goto L71;
        case 2: goto L83;
        default: break;
    }
L81:if (stick == 0) goto L69;                       // 81      IF(STICK.EQ.0) GOTO 69
    if (stick == 1) goto L82;                       //         IF(STICK.EQ.1)GOTO 82
    io.type(" "); io.type(stick);                   //         TYPE 68,STICK
    io.type(" OF THEM GET YOU.\n");                 // 68      FORMAT(' ',I2,' OF THEM GET YOU.',/)
    goto L83;                                       //         GOTO 83
L82:speak(6); // [6:"HE GETS YOU!"]                 // 82      CALL SPEAK(6)
L83:pause("GAMES OVER");                            // 83      PAUSE 'GAMES OVER'
    goto L71; // [or is it?]                        //         GOTO 71
L69:speak(7); // [7:"NONE OF THEM HIT YOU!"]        // 69      CALL SPEAK(7)
                                                    //
                                                    //       C PLACE DESCRIPTOR
                                                    //
                                                    // 
                                                    // 
L71:kk = stext.at(l);                               // 71      KK=STEXT(L)
    if (abb.at(l) == 0 || kk == 0) kk = ltext.at(l);//         IF(ABB(L).EQ.0.OR.KK.EQ.0)KK=LTEXT(L)
    if (kk == 0) goto L7;                           //         IF(KK.EQ.0) GOTO 7
L4:                                                 // 4       TYPE 5,(LLINE(KK,JJ),JJ=3,LLINE(KK,2))
                                                    // 5       FORMAT(20A5)
    scaffolding::type_20a5(io, lline.at(kk), 3, lline.at(kk)[2]);
    ++kk;                                           //         KK=KK+1
    if (lline.at(kk - 1)[1] != 0) goto L4;          //         IF(LLINE(KK-1,1).NE.0) GOTO 4
    io.type("\n");                                  //         TYPE 6
                                                    // 6       FORMAT(/)
L7: if (cond.at(l) == 2) goto L8;                   // 7       IF(COND(L).EQ.2)GOTO 8
                                                    //         IF(LOC.EQ.33.AND.RAN(QZ).LT.0.25)CALL SPEAK(8)
    if (loc == 33 && io.ran(7) < 0.25) speak(8);    // [8:"A HOLLOW VOICE SAYS 'PLUGH'"]
    j = l;                                          //         J=L
    goto L2000;                                     //         GOTO 2000
                                                    //
                                                    //       C GO GET A NEW LOCATION
                                                    //
L8: kk = key.at(loc);                               // 8       KK=KEY(LOC)
    if (kk == 0) goto L19;                          //         IF(KK.EQ.0)GOTO 19
    if (k == 57) goto L32;  // [57:LOOK]            //         IF(K.EQ.57)GOTO 32
    if (k == 67) goto L40;  // [67:CAVE]            //         IF(K.EQ.67)GOTO 40
    if (k ==  8) goto L12;  // [8:BACK]             //         IF(K.EQ.8)GOTO 12
    lold = l;                                       //         LOLD=L
L9: ll = travel.at(kk);                             // 9       LL=TRAVEL(KK)
    if (ll < 0) ll = -ll;                           //         IF(LL.LT.0) LL=-LL
    if (1 == (ll % 1024)) goto L10;                 //         IF(1.EQ.MOD(LL,1024))GOTO 10
    if (k == (ll % 1024)) goto L10;                 //         IF(K.EQ.MOD(LL,1024))GOTO 10
    if (travel.at(kk) < 0) goto L11;                //         IF(TRAVEL(KK).LT.0)GOTO 11
    ++kk;                                           //         KK=KK+1
    goto L9;                                        //         GOTO 9
L12:temp = lold;                                    // 12      TEMP=LOLD
    lold = l;                                       //         LOLD=L
    l = temp;                                       //         L=TEMP
    goto L21;                                       //         GOTO 21
L10:l = ll / 1024;                                  // 10      L=LL/1024
    goto L21;                                       //         GOTO 21
    // [12:"I DON'T KNOW HOW TO APPLY THAT WORD HERE."]
L11:jspk = 12;                                      // 11      JSPK=12
    // [43:EAST 44:WEST 45:NORTH 46:SOUTH 29:UP 30:DOWN
    //  9:"THERE IS NO WAY TO GO THAT DIRECTION."]
    if (k >= 43 && k <= 46) jspk = 9;               //         IF(K.GE.43.AND.K.LE.46)JSPK=9
    if (k == 29 || k == 30) jspk = 9;               //         IF(K.EQ.29.OR.K.EQ.30)JSPK=9
                                                    //         IF(K.EQ.7.OR.K.EQ.8.OR.K.EQ.36.OR.K.EQ.37.OR.K.EQ.68)
                                                    //       1 JSPK=10
    // [7:FORWA 8:BACK 36:LEFT 37:RIGHT 68:TURN
    //  10:"I AM UNSURE HOW YOU ARE FACING. USE COMPASS POINTS..."]
    if (k == 7 || k == 8 || k == 36 || k == 37 || k == 68) jspk = 10;
    // [11:OUT 19:IN 11:"I DON'T KNOW IN FROM OUT HERE. USE COMPASS POINTS..."]
    if (k == 11 || k == 19) jspk = 11;              //         IF(K.EQ.11.OR.K.EQ.19)JSPK=11
    // [59:"...I CANNOT TELL YOU WHERE REMOTE THINGS ARE."]
    if (jverb == 1) jspk = 59;                      //         IF(JVERB.EQ.1)JSPK=59
    // [48:XYZZY 42:"NOTHING HAPPENS."]
    if (k == 48) jspk = 42;                         //         IF(K.EQ.48)JSPK=42
    // [17:CRAWL 80:"WHICH WAY?"]
    if (k == 17) jspk = 80;                         //         IF(K.EQ.17)JSPK=80
    speak(jspk);                                    //         CALL SPEAK(JSPK)
    goto L2;                                        //         GOTO 2
L19:speak(13); // [13:"I DON'T UNDERSTAND THAT!"]   // 19      CALL SPEAK(13)
    l = loc;                                        //         L=LOC
    // [14:"I ALWAYS UNDERSTAND COMPASS DIRECTIONS,..."]
    if (ifirst == 0) speak(14);                     //         IF(IFIRST.EQ.0) CALL SPEAK(14)
L21:if (l < 300) goto L2;                           // 21      IF(L.LT.300)GOTO 2
    il = l - 300 + 1;                               //         IL=L-300+1
                                                    //         GOTO(22,23,24,25,26,31,27,28,29,30,33,34,36,37)IL
    switch (il) {
        case  1: goto L22; // [5 Forrest, "north"]
        case  2: goto L23; // [8 Outside grate, "down"]
        case  3: goto L24; // [9 Below grate, "exit"]
        case  4: goto L25; // [14 Top of small pit, "down"]
        case  5: goto L26; // [15 Hall of mists, "up"]
        case  6: goto L31; // [17 East bank of fissure, "onward"]
        case  7: goto L27; // [17 East bank of fissure, "jump"]
        case  8: goto L28; // [19 Hall of mt king, "north"]
        case  9: goto L29; // [19 Hall of mt king, "south"]
        case 10: goto L30; // [19 Hall of mt king, "west"]
        case 11: goto L33; // [11, 12, 13, 14, "depression"]
        case 12: goto L34; // [65 Bedquilt, "south"]
        case 13: goto L36; // [65 Bedquilt, "up"]
        case 14: goto L37; // [66 Swiss cheese room, "north"]
        // [Note: the computed goto in Crowther's code,GOTO(22,23,24,25,26,31,27,28,29,30,33,34,36,37)IL,
        //  does NOT include label 39. See L39 comment below.]
        case 15: goto L39; // [66 Swiss cheese room, "south" - this line is not in Crowther's code]
        default: break;
    }
    goto L2;                                        //         GOTO 2
                                                    //
L22:l = 6;                                          // 22      L=6
    if (io.ran(22) > 0.5) l = 5;                    //         IF(RAN(QZ).GT.0.5) L=5
    goto L2;                                        //         GOTO 2
L23:l = 23;                                         // 23      L=23
    if (prop[grate] != 0) l = 9;                    //         IF(PROP(GRATE).NE.0) L=9
    goto L2;                                        //         GOTO 2
L24:l = 9;                                          // 24      L=9
    if (prop[grate] != 0) l = 8;                    //         IF(PROP(GRATE).NE.0)L=8
    goto L2;                                        //         GOTO 2
L25:l = 20;                                         // 25      L=20
    if (iplace[nugget] != -1) l = 15;               //         IF(IPLACE(NUGGET).NE.-1)L=15
    // [Go into the pit carrying gold and you die! But there is a bug here: the map path
    //  becomes l=20,26,26,26... An infinite loop of "I DON'T UNDERSTAND THAT!"
    //  In this implementation I have added 'if (l == 26) pause("GAME OVER")' at L2 to stop this.]
    goto L2;                                        //         GOTO 2
L26:l = 22;                                         // 26      L=22
    if (iplace[nugget] != -1) l = 14;               //         IF(IPLACE(NUGGET).NE.-1) L=14
    goto L2;                                        //         GOTO 2
L27:l = 27;                                         // 27      L=27
    if (prop[12] == 0) l = 31; //[obj 12 is fissure]//         IF(PROP(12).EQ.0)L=31
    goto L2;                                        //         GOTO 2
L28:l = 28;                                         // 28      L=28
    if (prop[snake] == 0) l = 32;                   //         IF(PROP(SNAKE).EQ.0)L=32
    goto L2;                                        //         GOTO 2
L29:l = 29;                                         // 29      L=29
    if (prop[snake] == 0) l = 32;                   //         IF(PROP(SNAKE).EQ.0) L=32
    goto L2;                                        //         GOTO 2
L30:l = 30;                                         // 30      L=30
    if (prop[snake] == 0) l = 32;                   //         IF(PROP(SNAKE).EQ.0) L=32
    goto L2;                                        //         GOTO 2
L31:pause("GAME IS OVER");                          // 31      PAUSE 'GAME IS OVER'
    goto L1100;                                     //         GOTO 1100
    // [15:"SORRY, BUT I AM NOT ALLOWED TO GIVE MORE DETAIL..."]
L32:if (idetal < 3) speak(15);                      // 32      IF(IDETAL.LT.3)CALL SPEAK(15)
    ++idetal;                                       //         IDETAL=IDETAL+1
    l = loc;                                        //         L=LOC
    abb.at(l) = 0;                                  //         ABB(L)=0
    goto L2;                                        //         GOTO 2
L33:l = 8;                                          // 33      L=8
    if (prop[grate] == 0) l = 9;                    //         IF(PROP(GRATE).EQ.0) L=9
    goto L2;                                        //         GOTO 2
L34:if (io.ran(34) > 0.2) goto L35;                 // 34      IF(RAN(QZ).GT.0.2)GOTO 35
    l = 68;                                         //         L=68
    goto L2;                                        //         GOTO 2
L35:l = 65;                                         // 35      L=65
    // [56:"YOU HAVE CRAWLED AROUND IN SOME LITTLE HOLES AND WOUND UP BACK IN THE MAIN PASSAGE."]
L38:speak(56);                                      // 38      CALL SPEAK(56)
    goto L2;                                        //         GOTO 2
L36:if (io.ran(361) > 0.2) goto L35;                // 36      IF(RAN(QZ).GT.0.2)GOTO 35
    l = 39;                                         //         L=39
    if (io.ran(362) > 0.5) l = 70;                  //         IF(RAN(QZ).GT.0.5)L=70
    goto L2;                                        //         GOTO 2
L37:l = 66;                                         // 37      L=66
    if (io.ran(371) > 0.4) goto L38;                //         IF(RAN(QZ).GT.0.4)GOTO 38
    l = 71;                                         //         L=71
    if (io.ran(372) > 0.25) l = 72;                 //         IF(RAN(QZ).GT.0.25)L=72
    goto L2;                                        //         GOTO 2

// [There is no GOTO 39 in Crowther's code. The code at L39 would be unreachable.
//  Presumably, in the computed goto "GOTO(22,23,24,25,26,31,27,28,29,30,33,34,36,37)IL",
//  there should be a 39 added to the end. Because this is missing, going south in the
//  Swiss cheese room would cause an uncaught exception crash because l is still 314 at
//  goto L2. However, it seems reasonable to make the assumption that this is an oversight
//  and the code that Crowther wrote at L39 should be invoked in this situation.]
L39:l = 66;                                         // 39      L=66
    if (io.ran(39) > 0.2) goto L38;                 //         IF(RAN(QZ).GT.0.2)GOTO 38
    l = 77;                                         //         L=77
    goto L2;                                        //         GOTO 2

    // [57:"I DON'T KNOW WHERE THE CAVE IS,..."]
L40:if (loc < 8) speak(57);                         // 40      IF(LOC.LT.8)CALL SPEAK(57)
    // [58:"I NEED MORE DETAILED INSTRUCTIONS TO DO THAT."]
    if (loc >= 8) speak(58);                        //         IF(LOC.GE.8)CALL SPEAK(58)
    l = loc;                                        //         L=LOC
    goto L2;                                        //         GOTO 2
                                                    // 
                                                    // 
                                                    // 
                                                    //       C DO NEXT INPUT
                                                    // 
                                                    // 
L2000:ltrubl = 0;                                   // 2000    LTRUBL=0
    loc = j;                                        //         LOC=J
    abb.at(j) = (abb.at(j) + 1) % 5;                //         ABB(J)=MOD((ABB(J)+1),5)
    idark = 0;                                      //         IDARK=0
    if (cond.at(j) % 2 == 1) goto L2003;            //         IF(MOD(COND(J),2).EQ.1) GOTO 2003
    if (iplace[2]!=j && iplace[2]!=-1) goto L2001;  //         IF((IPLACE(2).NE.J).AND.(IPLACE(2).NE.-1)) GOTO 2001
    if (prop[2] == 1) goto L2003;                   //         IF(PROP(2).EQ.1)GOTO 2003
    // [16:"IT IS NOW PITCH BLACK. IF YOU PROCEED YOU WILL LIKELY FALL INTO A PIT."]
L2001:speak(16);                                    // 2001    CALL SPEAK(16)
    idark = 1;                                      //         IDARK=1
                                                    // 
                                                    // 
L2003:i = iobj.at(j);                               // 2003    I=IOBJ(J)
L2004:if (i == 0) goto L2011;                       // 2004    IF(I.EQ.0) GOTO 2011
    if ((i==6||i==9) && iplace[10]==-1) goto L2008; //         IF(((I.EQ.6).OR.(I.EQ.9)).AND.(IPLACE(10).EQ.-1))GOTO 2008
    ilk = i;                                        //         ILK=I
    if (prop.at(i) != 0) ilk = i + 100;             //         IF(PROP(I).NE.0) ILK=I+100
    kk = btext.at(ilk);                             //         KK=BTEXT(ILK)
    if (kk == 0) goto L2008;                        //         IF(KK.EQ.0) GOTO 2008
L2005:                                              // 2005    TYPE 2006,(LLINE(KK,JJ),JJ=3,LLINE(KK,2))
                                                    // 2006    FORMAT(20A5)
    scaffolding::type_20a5(io, lline.at(kk), 3, lline.at(kk)[2]);
    ++kk;                                           //         KK=KK+1
    if (lline.at(kk-1)[1] != 0) goto L2005;         //         IF(LLINE(KK-1,1).NE.0) GOTO 2005
    io.type("\n");                                  //         TYPE 2007
                                                    // 2007    FORMAT(/)
L2008:i = ichain.at(i);                             // 2008    I=ICHAIN(I)
    goto L2004;                                     //         GOTO 2004
                                                    // 
                                                    // 
                                                    // 
                                                    //       C K=1 MEANS ANY INPUT
                                                    // 
                                                    // 
L2012:a = wd2;                                      // 2012    A=WD2
    b = scaffolding::a5_space;                      //         B=' '
    twowds = 0;                                     //         TWOWDS=0
    goto L2021;                                     //         GOTO 2021
                                                    //
L2009:k = 54; // [54:"OK"]                          // 2009    K=54
L2010:jspk = k;                                     // 2010    JSPK=K
L5200:speak(jspk);                                  // 5200    CALL SPEAK(JSPK)
                                                    //
L2011:jverb = 0;                                    // 2011    JVERB=0
    jobj = 0;                                       //         JOBJ=0
    twowds = 0;                                     //         TWOWDS=0
                                                    //
L2020:getin(io, twowds, a, wd2, b);                 // 2020    CALL GETIN(TWOWDS,A,WD2,B)
    k = 70; // [70:"YOUR FEET ARE NOW WET."]        //         K=70
                                                    //         IF(A.EQ.'ENTER'.AND.(WD2.EQ.'STREA'.OR.WD2.EQ.'WATER'))GOTO 2010
    if (a == scaffolding::as_a5("ENTER") && (wd2 == scaffolding::as_a5("STREA") || wd2 == scaffolding::as_a5("WATER"))) goto L2010;
                                                    //         IF(A.EQ.'ENTER'.AND.TWOWDS.NE.0)GOTO 2012
    if (a == scaffolding::as_a5("ENTER") && twowds) goto L2012;
L2021:if (a!=scaffolding::as_a5("WEST")) goto L2023;// 2021    IF(A.NE.'WEST')GOTO 2023
    ++iwest;                                        //         IWEST=IWEST+1
    if (iwest != 10) goto L2023;                    //         IF(IWEST.NE.10)GOTO 2023
    // [17:"IF YOU PREFER, SIMPLY TYPE W RATHER THAN WEST."]
    speak(17);                                      //         CALL SPEAK(17)
L2023:for (i = 1; i <= 1000; ++i) {                 // 2023    DO 2024 I=1,1000
        if (ktab[i] == -1) goto L3000;              //         IF(KTAB(I).EQ.-1)GOTO 3000
        if (atab[i] == a) goto L2025;               //         IF(ATAB(I).EQ.A)GOTO 2025
    }                                               // 2024    CONTINUE
    pause("ERROR 6");                               //         PAUSE 'ERROR 6'
L2025:k = ktab.at(i) % 1000;                        // 2025    K=MOD(KTAB(I),1000)
    kq = ktab.at(i) / 1000 + 1;                     //         KQ=KTAB(I)/1000+1
    switch (kq) {                                   //         GOTO (5014,5000,2026,2010)KQ
        case 1: goto L5014; // [process movement]
        case 2: goto L5000; // [process noun]
        case 3: goto L2026; // [process verb]
        case 4: goto L2010; // [give advice only]
        default: break;
    }
    pause("NO NO");                                 //         PAUSE 'NO NO'
L2026:jverb = k;                                    // 2026    JVERB=K
    jspk = jspkt.at(jverb);                         //         JSPK=JSPKT(JVERB)
    if (twowds != 0) goto L2028;                    //         IF(TWOWDS.NE.0)GOTO 2028
    if (jobj == 0) goto L2036;                      //         IF(JOBJ.EQ.0)GOTO 2036
L2027:switch (jverb) {                              // 2027    GOTO(9000,5066,3000,5031,2009,5031,9404,9406,5081,5200,
        case  1: goto L9000; // [take]              //       1 5200,5300,5506,5502,5504,5505)JVERB
        case  2: goto L5066; // [drop]
        case  3: goto L3000; // [dummy]
        case  4: goto L5031; // [open]
        case  5: goto L2009; // [hold]
        case  6: goto L5031; // [lock]
        case  7: goto L9404; // [on]
        case  8: goto L9406; // [off]
        case  9: goto L5081; // [strike]
        case 10: goto L5200; // [calm]
        case 11: goto L5200; // [go]
        case 12: goto L5300; // [hit]
        case 13: goto L5506; // [pour]
        case 14: goto L5502; // [eat]
        case 15: goto L5504; // [drink]
        case 16: goto L5505; // [rub]
        default: break;
    }
    pause("ERROR 5");                               //         PAUSE 'ERROR 5'
                                                    // 
                                                    // 
L2028:a = wd2;                                      // 2028    A=WD2
    b = scaffolding::a5_space;                      //         B=' '
    twowds = 0;                                     //         TWOWDS=0
    goto L2023;                                     //         GOTO 2023
                                                    // 
    // [60:"I DON'T KNOW THAT WORD." 61:"WHAT?" 13:"I DON'T UNDERSTAND THAT!"]
L3000:jspk = 60;                                    // 3000    JSPK=60
    if (io.ran(30001) > 0.8) jspk = 61;             //         IF(RAN(QZ).GT.0.8)JSPK=61
    if (io.ran(30002) > 0.8) jspk = 13;             //         IF(RAN(QZ).GT.0.8)JSPK=13
    speak(jspk);                                    //         CALL SPEAK(JSPK)
    ++ltrubl;                                       //         LTRUBL=LTRUBL+1
    if (ltrubl != 3) goto L2020;                    //         IF(LTRUBL.NE.3)GOTO 2020
                                                    //         IF(J.NE.13.OR.IPLACE(7).NE.13.OR.IPLACE(5).NE.-1)GOTO 2032
    if (j != 13 || iplace[7] != 13 || iplace[5] != -1) goto L2032;
    // [18:"ARE YOU TRYING TO CATCH THE BIRD?" 19:"THE BIRD IS FRIGHTENED RIGHT NOW AND YOU CANNOT CATCH IT" 54:"OK"]
    yes(18, 19, 54, yea);                           //         CALL YES(18,19,54,YEA)
    goto L2033;                                     //         GOTO 2033
L2032:                                              // 2032    IF(J.NE.19.OR.PROP(11).NE.0.OR.IPLACE(7).EQ.-1)GOTO 2034
    if (j != 19 || prop[11] != 0 || iplace[7] == -1) goto L2034;
    // [20:"ARE YOU TRYING TO ATTACK OR AVOID THE SNAKE?" 21:"YOU CAN'T KILL THE SNAKE..." 54:"OK"]
    yes(20, 21, 54, yea);                           //         CALL YES(20,21,54,YEA)
    goto L2033;                                     //         GOTO 2033
L2034:if (j != 8 || prop[grate] != 0) goto L2035;   // 2034    IF(J.NE.8.OR.PROP(GRATE).NE.0)GOTO 2035
    // [62:"ARE YOU TRYING TO GET INTO THE CAVE?" 63:"THE GRATE IS VERY SOLID..." 54:"OK"]
    yes(62, 63, 54, yea);                           //         CALL YES(62,63,54,YEA)
L2033:if (yea == 0) goto L2011;                     // 2033    IF(YEA.EQ.0)GOTO 2011
    goto L2020;                                     //         GOTO 2020
L2035:if (iplace[5]!=j && iplace[5]!=-1) goto L2020;// 2035    IF(IPLACE(5).NE.J.AND.IPLACE(5).NE.-1)GOTO 2020
    if (jobj != 5) goto L2020;                      //         IF(JOBJ.NE.5)GOTO 2020
    // [22:"MY WORD FOR HITTING SOMETHING WITH THE ROD IS 'STRIKE'."]
    speak(22);                                      //         CALL SPEAK(22)
    goto L2020;                                     //         GOTO 2020
                                                    // 
                                                    // 
L2036:switch (jverb) {                              // 2036    GOTO(2037,5062,5062,9403,2009,9403,9404,9406,5062,5062,
        case  1: goto L2037;                        //       1 5200,5300,5062,5062,5062,5062)JVERB
        case  2: goto L5062;
        case  3: goto L5062;
        case  4: goto L9403;
        case  5: goto L2009;
        case  6: goto L9403;
        case  7: goto L9404;
        case  8: goto L9406;
        case  9: goto L5062;
        case 10: goto L5062;
        case 11: goto L5200;
        case 12: goto L5300;
        case 13: goto L5062;
        case 14: goto L5062;
        case 15: goto L5062;
        case 16: goto L5062;
        default: break;
    }
    pause("OOPS");                                  //         PAUSE 'OOPS'
                                                    // 2037    IF((IOBJ(J).EQ.0).OR.(ICHAIN(IOBJ(J)).NE.0)) GOTO 5062
L2037:if (iobj.at(j) == 0 || ichain.at(iobj[j]) != 0) goto L5062;
    for (i = 1; i <= 3; ++i) {                      //         DO 5312 I=1,3
        if (dseen[i] != 0) goto L5062;              //         IF(DSEEN(I).NE.0)GOTO 5062
    }                                               // 5312    CONTINUE
    jobj = iobj[j];                                 //         JOBJ=IOBJ(J)
    goto L2027;                                     //         GOTO 2027
L5062:if (b != scaffolding::a5_space) goto L5333;   // 5062    IF(B.NE.' ')GOTO 5333
                                                    //         TYPE 5063,A
                                                    // 5063    FORMAT('  ',A5,' WHAT?',/)
    io.type("  "); io.type(scaffolding::as_string(a)); io.type(" WHAT?\n");
    goto L2020;                                     //         GOTO 2020
                                                    //
L5333:                                              // 5333    TYPE 5334,A,B
                                                    // 5334    FORMAT(' ',2A5,' WHAT?',/)
    io.type(" "); io.type(scaffolding::as_string(a)); io.type(scaffolding::as_string(b)); io.type(" WHAT?\n");
    goto L2020;                                     //         GOTO 2020
L5014:if (idark == 0) goto L8;                      // 5014    IF(IDARK.EQ.0) GOTO 8
                                                    //
    if (io.ran(5014) > 0.25) goto L8;               //         IF(RAN(QZ).GT.0.25) GOTO 8
    // [23:"YOU FELL INTO A PIT AND BROKE EVERY BONE IN YOUR BODY!"]
    speak(23);                                      // 5017    CALL SPEAK(23)
    pause("GAME IS OVER");                          //         PAUSE 'GAME IS OVER'
    goto L2011;                                     //         GOTO 2011
    // [Interesting that rather than choosing to STOP Crowther allowed the
    // player to keep going with a deliberate GOTO 2011 on return from PAUSE.]
                                                    // 
                                                    // 
                                                    // 
L5000:jobj = k;                                     // 5000    JOBJ=K
    if (twowds != 0) goto L2028;                    //         IF(TWOWDS.NE.0)GOTO 2028
                                                    //         IF((J.EQ.IPLACE(K)).OR.(IPLACE(K).EQ.-1)) GOTO 5004
    if (j == iplace.at(k) || iplace[k] == -1) goto L5004;
    if (k != grate) goto L502;                      //         IF(K.NE.GRATE)GOTO 502
    if (j == 1 || j == 4 || j == 7) goto L5098;     //         IF((J.EQ.1).OR.(J.EQ.4).OR.(J.EQ.7))GOTO 5098
    if (j > 9 && j < 15) goto L5097;                //         IF((J.GT.9).AND.(J.LT.15))GOTO 5097
L502:if (b != scaffolding::a5_space) goto L5316;    // 502     IF(B.NE.' ')GOTO 5316
                                                    //         TYPE 5005,A
                                                    // 5005    FORMAT(' I SEE NO ',A5,' HERE.',/)
    io.type(" I SEE NO ");
    io.type(scaffolding::as_string(a));
    io.type(" HERE.\n");
    goto L2011;                                     //         GOTO 2011
L5316:                                              // 5316    TYPE 5317,A,B
                                                    // 5317    FORMAT(' I SEE NO ',2A5,' HERE.'/)
    io.type(" I SEE NO ");
    io.type(scaffolding::as_string(a));
    io.type(scaffolding::as_string(b));
    io.type(" HERE.\n");
    goto L2011;                                     //         GOTO 2011
L5098:k = 49;                                       // 5098    K=49
    goto L5014;                                     //         GOTO 5014
L5097:k = 50;                                       // 5097    K=50
    goto L5014;                                     //         GOTO 5014
L5004:jobj = k;                                     // 5004    JOBJ=K
    if (jverb != 0) goto L2027;                     //         IF(JVERB.NE.0)GOTO 2027
                                                    // 
                                                    // 
    if (b != scaffolding::a5_space) goto L5314;     // 5064    IF(B.NE.' ')GOTO 5314
                                                    //         TYPE 5001,A
                                                    // 5001    FORMAT(' WHAT DO YOU WANT TO DO WITH THE ',A5,'?',/)
    io.type(" WHAT DO YOU WANT TO DO WITH THE ");
    io.type(scaffolding::as_string(a));
    io.type("?\n");
    goto L2020;                                     //         GOTO 2020
L5314:                                              // 5314    TYPE 5315,A,B
                                                    // 5315    FORMAT(' WHAT DO YOU WANT TO DO WITH THE ',2A5,'?',/)
    io.type(" WHAT DO YOU WANT TO DO WITH THE ");
    io.type(scaffolding::as_string(a));
    io.type(scaffolding::as_string(b));
    io.type("?\n");
    goto L2020;                                     //         GOTO 2020
                                                    //
                                                    //       C CARRY
                                                    //
L9000:if (jobj == 18) goto L2009;                   // 9000    IF(JOBJ.EQ.18)GOTO 2009
    if (iplace.at(jobj) != j) goto L5200;           //         IF(IPLACE(JOBJ).NE.J) GOTO 5200
    if (ifixed.at(jobj) == 0) goto L9002;           // 9001    IF(IFIXED(JOBJ).EQ.0)GOTO 9002
    speak(25); // [25:"YOU CAN'T BE SERIOUS!"]      //         CALL SPEAK(25)
    goto L2011;                                     //         GOTO 2011
L9002:if (jobj != bird) goto L9004;                 // 9002    IF(JOBJ.NE.BIRD)GOTO 9004
    if (iplace[rod] != -1) goto L9003;              //         IF(IPLACE(ROD).NE.-1)GOTO 9003
    // [26:"THE BIRD WAS UNAFRAID WHEN YOU ENTERED, BUT AS YOU APPROACH IT BECOMES DISTURBED AND YOU CANNOT CATCH IT."]
    speak(26);                                      //         CALL SPEAK(26)
    goto L2011;                                     //         GOTO 2011
L9003:if (iplace[4]==-1 || iplace[4]==j) goto L9004;// 9003    IF((IPLACE(4).EQ.-1).OR.(IPLACE(4).EQ.J)) GOTO 9004
    // [27:"YOU CAN CATCH THE BIRD, BUT YOU CANNOT CARRY IT."]
    speak(27);                                      //         CALL SPEAK(27)
    goto L2011;                                     //         GOTO 2011
L9004:iplace.at(jobj) = -1; // [-1 means holding]   // 9004    IPLACE(JOBJ)=-1
L9005:if (iobj.at(j) != jobj) goto L9006;           // 9005    IF(IOBJ(J).NE.JOBJ) GOTO 9006
    iobj.at(j) = ichain.at(jobj);                   //         IOBJ(J)=ICHAIN(JOBJ)
    goto L2009;                                     //         GOTO 2009
L9006:itemp = iobj.at(j);                           // 9006    ITEMP=IOBJ(J)
L9007:if (ichain.at(itemp) == jobj) goto L9008;     // 9007    IF(ICHAIN(ITEMP).EQ.(JOBJ)) GOTO 9008
    itemp = ichain.at(itemp);                       //         ITEMP=ICHAIN(ITEMP)
    goto L9007;                                     //         GOTO 9007
L9008:ichain.at(itemp) = ichain.at(jobj);           // 9008    ICHAIN(ITEMP)=ICHAIN(JOBJ)
    goto L2009;                                     //         GOTO 2009
                                                    // 
                                                    // 
                                                    //       C LOCK, UNLOCK, NO OBJECT YET
                                                    //
L9403:if (j == 8 || j == 9) goto L5105;             // 9403    IF((J.EQ.8).OR.(J.EQ.9))GOTO 5105
    // [28:"THERE IS NOTHING HERE WITH A LOCK!"]
    speak(28);                                      // 5032    CALL SPEAK(28)
    goto L2011;                                     //         GOTO 2011
L5105:jobj = grate;                                 // 5105    JOBJ=GRATE
    goto L2027;                                     //         GOTO 2027
                                                    //
                                                    //       C DISCARD OBJECT
                                                    //
L5066:if (jobj == 18) goto L2009;                   // 5066    IF(JOBJ.EQ.18)GOTO 2009
    if (iplace.at(jobj) != -1) goto L5200;          //         IF(IPLACE(JOBJ).NE.-1) GOTO 5200
                                                    // 5012    IF((JOBJ.NE.BIRD).OR.(J.NE.19).OR.(PROP(11).EQ.1))GOTO 9401
    if (jobj != bird || j != 19 || prop[11] == 1) goto L9401;
    // [30:"THE LITTLE BIRD ATTACKS THE GREEN SNAKE, AND IN AN ASTOUNDING FLURRY DRIVES THE SNAKE AWAY."]
    speak(30);                                      //         CALL SPEAK(30)
    prop[11] = 1;                                   //         PROP(11)=1
L5160:ichain.at(jobj) = iobj.at(j);                 // 5160    ICHAIN(JOBJ)=IOBJ(J)
    iobj.at(j) = jobj;                              //         IOBJ(J)=JOBJ
    iplace.at(jobj) = j;                            //         IPLACE(JOBJ)=J
    goto L2011;                                     //         GOTO 2011
                                                    //
L9401:speak(54); // [54:"OK"]                       // 9401    CALL SPEAK(54)
    goto L5160;                                     //         GOTO 5160
                                                    //
                                                    //       C LOCK,UNLOCK OBJECT
                                                    //
                                                    // 5031    IF(IPLACE(KEYS).NE.-1.AND.IPLACE(KEYS).NE.J)GOTO 5200
L5031:if (iplace[keys] != -1 && iplace[keys] != j) goto L5200;
    if (jobj != 4) goto L5102;                      //         IF(JOBJ.NE.4)GOTO 5102
    speak(32); // [32:"IT HAS NO LOCK."]            //         CALL SPEAK(32)
    goto L2011;                                     //         GOTO 2011
L5102:if (jobj != keys) goto L5104;                 // 5102    IF(JOBJ.NE.KEYS)GOTO 5104
    speak(55); // [55:"YOU CAN'T UNLOCK THE KEYS."] //         CALL SPEAK(55)
    goto L2011;                                     //         GOTO 2011
L5104:if (jobj == grate) goto L5107;                // 5104    IF(JOBJ.EQ.GRATE)GOTO 5107
    // [33:"I DON'T KNOW HOW TO LOCK OR UNLOCK SUCH A THING."]
    speak(33);                                      //         CALL SPEAK(33)
    goto L2011;                                     //         GOTO 2011
L5107:if (jverb == 4) goto L5033;                   // 5107    IF(JVERB.EQ.4) GOTO 5033
    if (prop[grate] != 0) goto L5034;               //         IF(PROP(GRATE).NE.0)GOTO 5034
    // [34:"THE GRATE WAS ALREADY LOCKED."]
    speak(34);                                      //         CALL SPEAK(34)
    goto L2011;                                     //         GOTO 2011
L5034:speak(35); // [35:"THE GRATE IS NOW LOCKED."] // 5034    CALL SPEAK(35)
    prop[grate] = 0; // [0 means locked!]           //         PROP(GRATE)=0
    prop[8] = 0;                                    //         PROP(8)=0
    goto L2011;                                     //         GOTO 2011
L5033:if (prop[grate] == 0) goto L5109;             // 5033    IF(PROP(GRATE).EQ.0)GOTO 5109
    // [36:"THE GRATE WAS ALREADY UNLOCKED."]
    speak(36);                                      //         CALL SPEAK(36)
    goto L2011;                                     //         GOTO 2011
    // [37:"THE GRATE IS NOW UNLOCKED."]
L5109:speak(37);                                    // 5109    CALL SPEAK(37)
    prop[grate] = 1; // [1 means unlocked]          //         PROP(GRATE)=1
    prop[8] = 1;                                    //         PROP(8)=1
    goto L2011;                                     //         GOTO 2011
                                                    // 
                                                    // 
                                                    // 
                                                    //       C LIGHT LAMP
                                                    //
                                                    // 9404    IF((IPLACE(2).NE.J).AND.(IPLACE(2).NE.-1))GOTO 5200
L9404:if (iplace[2] != j && iplace[2] != -1) goto L5200;
    prop[2] = 1;                                    //         PROP(2)=1
    idark = 0;                                      //         IDARK=0
    speak(39); // [39:"YOUR LAMP IS NOW ON."]       //         CALL SPEAK(39)
    goto L2011;                                     //         GOTO 2011
                                                    //
                                                    //       C LAMP OFF
                                                    //
                                                    // 9406    IF((IPLACE(2).NE.J).AND.(IPLACE(2).NE.-1)) GOTO 5200
L9406:if (iplace[2] != j && iplace[2] != -1) goto L5200;
    prop[2] = 0;                                    //         PROP(2)=0
    speak(40); // [40:"YOUR LAMP IS NOW OFF."]      //         CALL SPEAK(40)
    goto L2011;                                     //         GOTO 2011
                                                    //
                                                    //       C STRIKE
                                                    //
L5081:if (jobj != 12) goto L5200;                   // 5081    IF(JOBJ.NE.12)GOTO 5200
    // [Strike the fissure (object 12) with the rod and a crystal bridge apears!]
    prop[12] = 1;                                   //         PROP(12)=1
    goto L2003;                                     //         GOTO 2003
                                                    //
                                                    //       C ATTACK
                                                    //
L5300:for (id = 1; id <= 3; ++id) {                 // 5300    DO 5313 ID=1,3
        iid = id;                                   //         IID=ID
        if (dseen[id] != 0) goto L5307;             //         IF(DSEEN(ID).NE.0)GOTO 5307
    }                                               // 5313    CONTINUE
    if (jobj == 0) goto L5062;                      //         IF(JOBJ.EQ.0)GOTO 5062
    if (jobj == snake) goto L5200;                  //         IF(JOBJ.EQ.SNAKE) GOTO 5200
    if (jobj == bird) goto L5302;                   //         IF(JOBJ.EQ.BIRD) GOTO 5302
    // [44:"THERE IS NOTHING HERE TO ATTACK."]
    speak(44);                                      //         CALL SPEAK(44)
    goto L2011;                                     //         GOTO 2011
    // [45:"THE LITTLE BIRD IS NOW DEAD. ITS BODY DISAPPEARS."]
L5302:speak(45);                                    // 5302    CALL SPEAK(45)
    iplace.at(jobj) = 300;                          //         IPLACE(JOBJ)=300
    goto L9005;                                     //         GOTO 9005
                                                    //
L5307:if (io.ran(5307) > 0.4) goto L5309;           // 5307    IF(RAN(QZ).GT.0.4) GOTO 5309
    dseen.at(iid) = 0;                              //         DSEEN(IID)=0
    odloc.at(iid) = 0;                              //         ODLOC(IID)=0
    dloc.at(iid) = 0;                               //         DLOC(IID)=0
    speak(47); // [47:"YOU KILLED A LITTLE DWARF."] //         CALL SPEAK(47)
    goto L5311;                                     //         GOTO 5311
    // [48:"YOU ATTACK A LITTLE DWARF, BUT HE DODGES OUT OF THE WAY."]
L5309:speak(48);                                    // 5309    CALL SPEAK(48)
L5311:k = 21;                                       // 5311    K=21
    goto L5014;                                     //         GOTO 5014
                                                    //
                                                    //       C EAT
                                                    //
                                                    // 5502    IF((IPLACE(FOOD).NE.J.AND.IPLACE(FOOD).NE.-1).OR.PROP(FOOD).NE.0
                                                    //       1 .OR.JOBJ.NE.FOOD)GOTO 5200
L5502:if ((iplace[food] != j && iplace[food] != -1) || prop[food] != 0 || jobj != food) goto L5200;
    prop[food] = 1;                                 //         PROP(FOOD)=1
    jspk = 72; // [72:"EATEN!"]                     // 5501    JSPK=72
    goto L5200;                                     //         GOTO 5200
                                                    //
                                                    //       C DRINK
                                                    //
                                                    // 5504    IF((IPLACE(WATER).NE.J.AND.IPLACE(WATER).NE.-1)
                                                    //       1 .OR.PROP(WATER).NE.0.OR.JOBJ.NE.WATER) GOTO 5200
L5504:if ((iplace[water] != j && iplace[water] != -1) || prop[water] != 0 || jobj != water) goto L5200;
    prop[water] = 1;                                //         PROP(WATER)=1
    // [74:"THE BOTTLE OF WATER IS NOW EMPTY."]
    jspk = 74;                                      //         JSPK=74
    goto L5200;                                     //         GOTO 5200
                                                    //
                                                    //       C RUB
                                                    //
    // [76:"PECULIAR.  NOTHING UNEXPECTED HAPPENS."]
L5505:if (jobj != lamp) jspk = 76;                  // 5505    IF(JOBJ.NE.LAMP)JSPK=76
    goto L5200;                                     //         GOTO 5200
                                                    //
                                                    //       C POUR
                                                    //
    // [78:"YOU CAN'T POUR THAT."]
L5506:if (jobj != water) jspk = 78;                 // 5506    IF(JOBJ.NE.WATER)JSPK=78
    prop[water] = 1;                                //         PROP(WATER)=1
    goto L5200;                                     //         GOTO 5200
                                                    // 
                                                    // 
                                                    // 
}                                                   //         END





/*
       ###    ########  ##     ## ########     ###    ######## 
      ## ##   ##     ## ##     ## ##     ##   ## ##      ##    
     ##   ##  ##     ## ##     ## ##     ##  ##   ##     ##    
    ##     ## ##     ## ##     ## ##     ## ##     ##    ##    
    ######### ##     ##  ##   ##  ##     ## #########    ##    
    ##     ## ##     ##   ## ##   ##     ## ##     ##    ##    
    ##     ## ########     ###    ########  ##     ##    ##    
*/

// The contents of the text file http://www.icynic.com/~don/jerz/advdat.77-03-31
const std::string advdat_77_03_31{
    "1\n"
    "1    YOU ARE STANDING AT THE END OF A ROAD BEFORE A SMALL BRICK\n"
    "1    BUILDING . AROUND YOU IS A FOREST. A SMALL\n"
    "1    STREAM FLOWS OUT OF THE BUILDING AND DOWN A GULLY.\n"
    "2    YOU HAVE WALKED UP A HILL, STILL IN THE FOREST\n"
    "2    THE ROAD NOW SLOPES BACK DOWN THE OTHER SIDE OF THE HILL.\n"
    "2    THERE IS A BUILDING IN THE DISTANCE.\n"
    "3    YOU ARE INSIDE A BUILDING, A WELL HOUSE FOR A LARGE SPRING.\n"
    "4    YOU ARE IN A VALLEY IN THE FOREST BESIDE A STREAM TUMBLING\n"
    "4    ALONG A ROCKY BED.\n"
    "5    YOU ARE IN OPEN FOREST, WITH A DEEP VALLEY TO ONE SIDE.\n"
    "6    YOU ARE IN OPEN FOREST NEAR BOTH A VALLEY AND A ROAD.\n"
    "7    AT YOUR FEET ALL THE WATER OF THE STREAM SPLASHES INTO A\n"
    "7    2 INCH SLIT IN THE ROCK. DOWNSTREAM THE STREAMBED IS BARE ROCK.\n"
    "8    YOU ARE IN A 20 FOOT DEPRESSION FLOORED WITH BARE DIRT. SET INTO\n"
    "8    THE DIRT IS A STRONG STEEL GRATE MOUNTED IN CONCRETE. A DRY\n"
    "8    STREAMBED LEADS INTO THE DEPRESSION.\n"
    "9    YOU ARE IN A SMALL CHAMBER BENEATH A 3X3 STEEL GRATE TO THE\n"
    "9    SURFACE. A LOW CRAWL OVER COBBLES LEADS INWARD TO THE WEST.\n"
    "10   YOU ARE CRAWLING OVER COBBLES IN A LOW PASSAGE. THERE IS A\n"
    "10   DIM LIGHT AT THE EAST END OF THE PASSAGE.\n"
    "11   YOU ARE IN A DEBRIS ROOM, FILLED WITH STUFF WASHED IN FROM\n"
    "11   THE SURFACE. A LOW WIDE PASSAGE WITH COBBLES BECOMES\n"
    "11   PLUGGED WITH MUD AND DEBRIS HERE,BUT AN AWKWARD CANYON\n"
    "11   LEADS UPWARD AND WEST.\n"
    "11   A NOTE ON THE WALL SAYS 'MAGIC WORD XYZZY'.\n"
    "12   YOU ARE IN AN AWKWARD SLOPING EAST/WEST CANYON.\n"
    "13   YOU ARE IN A SPLENDID CHAMBER THIRTY FEET HIGH. THE WALLS\n"
    "13   ARE FROZEN RIVERS OF ORANGE STONE. AN AWKWARD CANYON AND A\n"
    "13   GOOD PASSAGE EXIT FROM EAST AND WEST SIDES OF THE CHAMBER.\n"
    "14   AT YOUR FEET IS A SMALL PIT BREATHING TRACES OF WHITE MIST. AN\n"
    "14   EAST PASSAGE ENDS HERE EXCEPT FOR A SMALL CRACK LEADING ON.\n"
    "15   YOU ARE AT ONE END OF A VAST HALL STRETCHING FORWARD OUT OF\n"
    "15   SIGHT TO THE WEST. THERE ARE OPENINGS TO EITHER SIDE. NEARBY, A WIDE\n"
    "15   STONE STAIRCASE LEADS DOWNWARD. THE HALL IS FILLED WITH\n"
    "15   WISPS OF WHITE MIST SWAYING TO AND FRO ALMOST AS IF ALIVE.\n"
    "15   A COLD WIND BLOWS UP THE STAIRCASE. THERE IS A PASSAGE\n"
    "15   AT THE TOP OF A DOME BEHIND YOU.\n"
    "16   THE CRACK IS FAR TOO SMALL FOR YOU TO FOLLOW.\n"
    "17   YOU ARE ON THE EAST BANK OF A FISSURE SLICING CLEAR ACROSS\n"
    "17   THE HALL. THE MIST IS QUITE THICK HERE, AND THE FISSURE IS\n"
    "17   TOO WIDE TO JUMP.\n"
    "18   THIS IS A LOW ROOM WITH A CRUDE NOTE ON THE WALL.\n"
    "18   IT SAYS 'YOU WON'T GET IT UP THE STEPS'.\n"
    "19   YOU ARE IN THE HALL OF THE MOUNTAIN KING, WITH PASSAGES\n"
    "19   OFF IN ALL DIRECTIONS.\n"
    "20   YOU ARE AT THE BOTTOM OF THE PIT WITH A BROKEN NECK.\n"
    "21   YOU DIDN'T MAKE IT\n"
    "22   THE DOME IS UNCLIMBABLE\n"
    "23   YOU CAN'T GO IN THROUGH A LOCKED STEEL GRATE!\n"
    "24   YOU DON'T FIT DOWN A TWO INCH HOLE!\n"
    "25   YOU CAN'T GO THROUGH A LOCKED STEEL GRATE.\n"
    "27   YOU ARE ON THE WEST SIDE OF THE FISSURE IN THE HALL OF MISTS.\n"
    "28   YOU ARE IN A LOW N/S PASSAGE AT A HOLE IN THE FLOOR.\n"
    "28   THE HOLE GOES DOWN TO AN E/W PASSAGE.\n"
    "29   YOU ARE IN THE SOUTH SIDE CHAMBER.\n"
    "30   YOU ARE IN THE WEST SIDE CHAMBER OF HALL OF MT KING.\n"
    "30   A PASSAGE CONTINUES WEST AND UP HERE.\n"
    "31   THERE IS NO WAY ACROSS THE FISSURE.\n"
    "32   YOU CAN'T GET BY THE SNAKE\n"
    "33   YOU ARE IN A LARGE ROOM, WITH A PASSAGE TO THE SOUTH,\n"
    "33   A PASSAGE TO THE WEST, AND A WALL OF BROKEN ROCK TO\n"
    "33   THE EAST. THERE IS A LARGE 'Y2' ON A ROCK IN ROOMS CENTER.\n"
    "34   YOU ARE IN A JUMBLE OF ROCK, WITH CRACKS EVERYWHERE.\n"
    "35   YOU ARE AT A WINDOW ON A HUGE PIT, WHICH GOES UP AND\n"
    "35   DOWN OUT OF SIGHT. A FLOOR IS INDISTINCTLY VISIBLE\n"
    "35   OVER 50 FEET BELOW. DIRECTLY OPPOSITE YOU AND 25 FEET AWAY\n"
    "35   THERE IS A SIMILAR WINDOW.\n"
    "36   YOU ARE IN A DIRTY BROKEN PASSAGE. TO THE EAST IS A CRAWL.\n"
    "36   TO THE WEST IS A LARGE PASSAGE. ABOVE YOU IS A HOLE TO\n"
    "36   ANOTHER PASSAGE.\n"
    "37   YOU ARE ON THE BRINK OF A SMALL CLEAN CLIMBABLE PIT.\n"
    "37   A CRAWL LEADS WEST.\n"
    "38   YOU ARE IN THE BOTTOM OF A SMALL PIT WITH A LITTLE\n"
    "38   STREAM, WHICH ENTERS AND EXITS THROUGH TINY SLITS.\n"
    "39   YOU ARE IN A LARGE ROOM FULL OF DUSTY ROCKS. THERE IS A\n"
    "39   BIG HOLE IN THE FLOOR. THERE ARE CRACKS EVERYWHERE, AND\n"
    "39   A PASSAGE LEADING EAST.\n"
    "40   YOU HAVE CRAWLED THROUGH A VERY LOW WIDE PASSAGE PARALLEL\n"
    "40   TO AND NORTH OF THE HALL OF MISTS.\n"
    "41   YOU ARE AT THE WEST END OF HALL OF MISTS. A LOW WIDE CRAWL\n"
    "41   CONTINUES WEST AND ANOTHER GOES NORTH. TO THE SOUTH IS A\n"
    "41   LITTLE PASSAGE 6 FEET OFF THE FLOOR.\n"
    "42   YOU ARE IN A MAZE OF TWISTY LITTLE PASSAGES, ALL ALIKE.\n"
    "43   YOU ARE IN A MAZE OF TWISTY LITTLE PASSAGES, ALL ALIKE.\n"
    "44   YOU ARE IN A MAZE OF TWISTY LITTLE PASSAGES, ALL ALIKE.\n"
    "45   YOU ARE IN A MAZE OF TWISTY LITTLE PASSAGES, ALL ALIKE.\n"
    "46   DEAD END\n"
    "47   DEAD END\n"
    "48   DEAD END\n"
    "49   YOU ARE IN A MAZE OF TWISTY LITTLE PASSAGES, ALL ALIKE.\n"
    "50   YOU ARE IN A MAZE OF TWISTY LITTLE PASSAGES, ALL ALIKE.\n"
    "51   YOU ARE IN A MAZE OF TWISTY LITTLE PASSAGES, ALL ALIKE.\n"
    "52   YOU ARE IN A MAZE OF TWISTY LITTLE PASSAGES, ALL ALIKE.\n"
    "53   YOU ARE IN A MAZE OF TWISTY LITTLE PASSAGES, ALL ALIKE.\n"
    "54   DEAD END\n"
    "55   YOU ARE IN A MAZE OF TWISTY LITTLE PASSAGES, ALL ALIKE.\n"
    "56   DEAD END\n"
    "57   YOU ARE ON THE BRINK OF A THIRTY FOOT PIT WITH A MASSIVE\n"
    "57   ORANGE COLUMN DOWN ONE WALL. YOU COULD CLIMB DOWN HERE\n"
    "57   BUT YOU COULD NOT GET BACK UP. THE MAZE CONTINUES AT THIS\n"
    "57   LEVEL.\n"
    "58   DEAD END\n"
    "59   YOU HAVE CRAWLED THROUGH A VERY LOW WIDE PASSAGE PARALLEL\n"
    "59   TO AND NORTH OF THE HALL OF MISTS.\n"
    "60   YOU ARE AT THE EAST END OF A VERY LONG HALL APPARENTLY\n"
    "60   WITHOUT SIDE CHAMBERS. TO THE EAST A LOW WIDE CRAWL SLANTS\n"
    "60   UP. TO THE NORTH A ROUND TWO FOOT HOLE SLANTS DOWN.\n"
    "61   YOU ARE AT THE WEST END OF A VERY LONG FEATURELESS HALL.\n"
    "62   YOU ARE AT A CROSSOVER OF A HIGH N/S PASSAGE AND A LOW E/W ONE.\n"
    "63   DEAD END\n"
    "64   YOU ARE AT A COMPLEX JUNCTION. A LOW HANDS AND KNEES\n"
    "64   PASSAGE FROM THE NORTH JOINS A HIGHER CRAWL\n"
    "64   FROM THE EAST TO MAKE  A WALKING PASSAGE GOING WEST\n"
    "64   THERE IS ALSO A LARGE ROOM ABOVE. THE AIR IS DAMP HERE.\n"
    "64   A SIGN IN MIDAIR HERE SAYS 'CAVE UNDER CONSTRUCTION BEYOND\n"
    "64   THIS POINT. PROCEED AT OWN RISK.'\n"
    "65   YOU ARE IN BEDQUILT, A LONG EAST/WEST PASSAGE WITH HOLES EVERYWHERE.\n"
    "65   TO EXPLORE AT RANDOM SELECT NORTH, SOUTH, UP, OR DOWN.\n"
    "66   YOU ARE IN A ROOM WHOSE WALLS RESEMBLE SWISS CHEESE.\n"
    "66   OBVIOUS PASSAGES GO WEST,EAST,NE, AND\n"
    "66   NW. PART OF THE ROOM IS OCCUPIED BY A LARGE BEDROCK BLOCK.\n"
    "67   YOU ARE IN THE TWOPIT ROOM. THE FLOOR\n"
    "67   HERE IS LITTERED WITH THIN ROCK SLABS, WHICH MAKE IT\n"
    "67   EASY TO DESCEND THE PITS. THERE IS A PATH HERE BYPASSING\n"
    "67   THE PITS TO CONNECT PASSAGES FROM EAST AND WEST.THERE\n"
    "67   ARE HOLES ALL OVER, BUT THE ONLY BIG ONE IS ON THE WALL\n"
    "67   DIRECTLY OVER THE EAST PIT WHERE YOU CAN'T GET TO IT.\n"
    "68   YOU ARE IN A LARGE LOW CIRCULAR CHAMBER WHOSE FLOOR IS AN\n"
    "68   IMMENSE SLAB FALLEN FROM THE CEILING(SLAB ROOM). EAST AND\n"
    "68   WEST THERE ONCE WERE LARGE PASSAGES, BUT THEY ARE NOW FILLED\n"
    "68   WITH BOULDERS. LOW SMALL PASSAGES GO NORTH AND SOUTH, AND THE\n"
    "68   SOUTH ONE QUICKLY BENDS WEST AROUND THE BOULDERS.\n"
    "69   YOU ARE IN A SECRET NS CANYON ABOVE A LARGE ROOM.\n"
    "70   YOU ARE IN A SECRET N/S CANYON ABOVE A SIZABLE PASSAGE.\n"
    "71   YOU ARE IN SECRET CANYON AT A JUNCTION OF THREE CANYONS,\n"
    "71   BEARING NORTH, SOUTH, AND SE. THE NORTH ONE IS AS TALL\n"
    "71   AS THE OTHER TWO COMBINED.\n"
    "72   YOU ARE IN A LARGE LOW ROOM. CRAWLS LEAD N, SE, AND SW.\n"
    "73   DEAD END CRAWL.\n"
    "74   YOU ARE IN SECRET CANYON WHICH HERE RUNS E/W. IT CROSSES OVER\n"
    "74   A VERY TIGHT CANYON 15 FEET BELOW. IF YOU GO DOWN YOU MAY\n"
    "74   NOT BE ABLE TO GET BACK UP\n"
    "75   YOU ARE AT A WIDE PLACE IN A VERY TIGHT N/S CANYON.\n"
    "76   THE CANYON HERE BECOMES TO TIGHT TO GO FURTHER SOUTH.\n"
    "77   YOU ARE IN A TALL E/W CANYON. A LOW TIGHT CRAWL GOES 3 FEET\n"
    "77   NORTH AND SEEMS TO OPEN UP.\n"
    "78   THE CANYON RUNS INTO A MASS OF BOULDERS - DEAD END.\n"
    "79   THE STREAM FLOWS OUT THROUGH A PAIR OF 1 FOOT DIAMETER SEWER\n"
    "79   PIPES. IT WOULD BE ADVISABLE TO USE THE DOOR.\n"
    "-1  END\n"
    "2\n"
    "1    YOU'RE AT END OF ROAD AGAIN.\n"
    "2    YOU'RE AT HILL IN ROAD.\n"
    "3    YOU'RE INSIDE BUILDING.\n"
    "4    YOU'RE IN VALLEY\n"
    "5    YOU'RE IN FOREST\n"
    "6    YOU'RE IN FOREST\n"
    "7    YOU'RE AT SLIT IN STREAMBED\n"
    "8    YOU'RE OUTSIDE GRATE\n"
    "9    YOU'RE BELOW THE GRATE\n"
    "10   YOU'RE IN COBBLE CRAWL\n"
    "11   YOU'RE IN DEBRIS ROOM.\n"
    "13   YOU'RE IN BIRD CHAMBER.\n"
    "14   YOU'RE AT TOP OF SMALL PIT.\n"
    "15   YOU'RE IN HALL OF MISTS.\n"
    "17   YOU'RE ON EAST BANK OF FISSURE.\n"
    "18   YOU'RE IN NUGGET OF GOLD ROOM.\n"
    "19   YOU'RE IN HALL OF MT KING.\n"
    "33   YOU'RE AT Y2\n"
    "35   YOU'RE AT WINDOW ON PIT\n"
    "36   YOU'RE IN DIRTY PASSAGE\n"
    "39   YOU'RE N DUSTY ROCK ROOM.\n"
    "41   YOU'RE AT WEST END OF HALL OF MISTS.\n"
    "57   YOU'RE AT BRINK OF PIT.\n"
    "60   YOU'RE AT EAST END OF LONG HALL.\n"
    "66   YOU'RE IN SWISS CHEESE ROOM\n"
    "67   YOU'RE IN TWOPIT ROOM\n"
    "68   YOU'RE IN SLAB ROOM\n"
    "-1\n"
    "3\n"
    "1   2   2   44\n"
    "1   3   3   12  19  43\n"
    "1   4   4   5   13  14  46  30\n"
    "1   5   6   45  43\n"
    "1   8   49\n"
    "2   1   8   2   12  7   43  45  30\n"
    "2   5   6   45  46\n"
    "3   1   3   11  32  44\n"
    "3   11  48\n"
    "3   33  65\n"
    "3   79  5   14\n"
    "4   1   4   45\n"
    "4   5   6   43  44  29\n"
    "4   7   5   46  30\n"
    "4   8   49\n"
    "5   4   9   43  30\n"
    "5   300 6   7   8   45\n"
    "5   5   44  46\n"
    "6   1   2   45\n"
    "6   4   9   43  44  30\n"
    "6   5   6   46\n"
    "7   1   12\n"
    "7   4   4   45\n"
    "7   5   6   43  44\n"
    "7   8   5   15  16  46  30\n"
    "7   24  47  14  30\n"
    "8   5   6   43  44  46\n"
    "8   1   12\n"
    "8   7   4   13  45\n"
    "8   301 3   5   19  30\n"
    "9   302 11  12\n"
    "9   10  17  18  19  44\n"
    "9   14  31\n"
    "9   11  51\n"
    "10  9   11  20  21  43\n"
    "10  11  19  22  44  51\n"
    "10  14  31\n"
    "11  310 49\n"
    "11  10  17  18  23  24  43\n"
    "11  12  25  305 19  29  44\n"
    "11  3   48\n"
    "11  14  31\n"
    "12  310 49\n"
    "12  11  30  43  51\n"
    "12  13  19  29  44\n"
    "12  14  31\n"
    "13  310 49\n"
    "13  11  51\n"
    "13  12  25  305 43\n"
    "13  14  23  31  44\n"
    "14  310 49\n"
    "14  11  51\n"
    "14  13  23  43\n"
    "14  303 30  31  34\n"
    "14  16  33  44\n"
    "15  18  36  46\n"
    "15  17  7   38  44\n"
    "15  19  10  30  45\n"
    "15  304 29  31  34  35  23  43\n"
    "15  34  55\n"
    "15  62  69\n"
    "16  14  1\n"
    "17  15  8   38  43\n"
    "17  305 7\n"
    "17  306 40  41  42  44  19  39\n"
    "18  15  38  11  8   45\n"
    "19  15  10  29  43\n"
    "19  307 45  36\n"
    "19  308 46  37\n"
    "19  309 44  7\n"
    "19  74  66\n"
    "20  26  1\n"
    "21  26  1\n"
    "22  15  1\n"
    "23  8   1\n"
    "24  7   1\n"
    "25  9   1\n"
    "27  17  8   11  38\n"
    "27  40  45\n"
    "27  41  44\n"
    "28  19  38  11  46\n"
    "28  33  45\n"
    "28  36  30  52\n"
    "29  19  38  11  45\n"
    "30  19  38  11  43\n"
    "30  62  44  29\n"
    "31  17  1\n"
    "32  19  1\n"
    "33  3   65\n"
    "33  28  46\n"
    "33  34  43  53  54\n"
    "33  35  44\n"
    "34  33  30\n"
    "34  15  29\n"
    "35  33  43  55\n"
    "36  37  43  17\n"
    "36  28  29  52\n"
    "36  39  44\n"
    "37  36  44  17\n"
    "37  38  30  31  56\n"
    "38  37  56  29\n"
    "39  36  43\n"
    "39  64  30  52  58\n"
    "39  65  70\n"
    "40  41  1\n"
    "41  42  46  29  23  56\n"
    "41  27  43\n"
    "41  59  45\n"
    "41  60  44  17\n"
    "42  41  44\n"
    "42  43  43\n"
    "42  44  46\n"
    "43  42  44\n"
    "43  44  46\n"
    "43  45  43\n"
    "44  42  45\n"
    "44  43  43\n"
    "44  48  30\n"
    "44  50  46\n"
    "45  43  45\n"
    "45  46  43\n"
    "45  47  46\n"
    "46  45  44  11\n"
    "47  45  45  11\n"
    "48  44  29  11\n"
    "49  50  30  43\n"
    "49  51  44\n"
    "50  44  43\n"
    "50  49  44  29\n"
    "50  52  46\n"
    "51  49  44\n"
    "51  52  43\n"
    "51  53  46\n"
    "52  50  45\n"
    "52  51  44\n"
    "52  53  29\n"
    "52  55  43\n"
    "53  51  44\n"
    "53  52  45\n"
    "53  54  46\n"
    "54  53  43  11\n"
    "55  52  44\n"
    "55  56  30\n"
    "55  57  43\n"
    "56  55  29  11\n"
    "57  55  44\n"
    "57  58  46\n"
    "57  13  30  56\n"
    "58  57  44  11\n"
    "59  27  1\n"
    "60  41  43  29\n"
    "60  61  44\n"
    "60  62  45  30\n"
    "61  60  43  11\n"
    "62  60  44\n"
    "62  63  45\n"
    "62  30  43\n"
    "62  15  46\n"
    "63  62  46  11\n"
    "64  39  29  56  59\n"
    "64  65  44\n"
    "65  64  43\n"
    "65  66  44\n"
    "65  68  61\n"
    "65  311 46\n"
    "65  312 29\n"
    "66  313 45\n"
    "66  65  60\n"
    "66  67  44\n"
    "66  77  25\n"
    "66  314 46\n"
    "67  66  43\n"
    "67  72  60\n"
    "68  66  46\n"
    "68  69  29\n"
    "69  68  30\n"
    "69  74  46\n"
    "70  71  45\n"
    "71  39  29\n"
    "71  65  62\n"
    "71  70  46\n"
    "72  67  63\n"
    "72  73  45\n"
    "73  72  46\n"
    "74  19  43\n"
    "74  69  44\n"
    "74  75  30\n"
    "75  76  46\n"
    "75  77  45\n"
    "76  75  45\n"
    "77  75  43\n"
    "77  78  44\n"
    "77  66  45\n"
    "78  77  46\n"
    "79  3   1\n"
    "-1\n"
    "4\n"
    "2   ROAD\n"
    "3   ENTER\n"
    "3   DOOR\n"
    "3   GATE\n"
    "4   UPSTR\n"
    "5   DOWNS\n"
    "6   FORES\n"
    "7   FORWA\n"
    "7   CONTI\n"
    "7   ONWAR\n"
    "8   BACK\n"
    "8   RETUR\n"
    "8   RETRE\n"
    "9   VALLE\n"
    "10  STAIR\n"
    "11  OUT\n"
    "11  OUTSI\n"
    "11  EXIT\n"
    "11  LEAVE\n"
    "12  BUILD\n"
    "12  BLD\n"
    "12  HOUSE\n"
    "13  GULLY\n"
    "14  STREA\n"
    "15  ROCK\n"
    "16  BED\n"
    "17  CRAWL\n"
    "18  COBBL\n"
    "19  INWAR\n"
    "19  INSID\n"
    "19  IN\n"
    "20  SURFA\n"
    "21  NULL\n"
    "21  NOWHE\n"
    "22  DARK\n"
    "23  PASSA\n"
    "24  LOW\n"
    "25  CANYO\n"
    "26  AWKWA\n"
    "29  UPWAR\n"
    "29  UP\n"
    "29  U\n"
    "29  ABOVE\n"
    "30  D\n"
    "30  DOWNW\n"
    "30  DOWN\n"
    "31  PIT\n"
    "32  OUTDO\n"
    "33  CRACK\n"
    "34  STEPS\n"
    "35  DOME\n"
    "36  LEFT\n"
    "37  RIGHT\n"
    "38  HALL\n"
    "39  JUMP\n"
    "40  MAGIC\n"
    "41  OVER\n"
    "42  ACROS\n"
    "43  EAST\n"
    "43  E\n"
    "44  WEST\n"
    "44  W\n"
    "45  NORTH\n"
    "45  N\n"
    "46  SOUTH\n"
    "46  S\n"
    "47  SLIT\n"
    "48  XYZZY\n"
    "49  DEPRE\n"
    "50  ENTRA\n"
    "51  DEBRI\n"
    "52  HOLE\n"
    "53  WALL\n"
    "54  BROKE\n"
    "55  Y2\n"
    "56  CLIMB\n"
    "57  LOOK\n"
    "57  EXAMI\n"
    "57  TOUCH\n"
    "57  LOOKA\n"
    "58  FLOOR\n"
    "59  ROOM\n"
    "60  NE\n"
    "61  SLAB\n"
    "61  SLABR\n"
    "62  SE\n"
    "63  SW\n"
    "64  NW\n"
    "65  PLUGH\n"
    "66  SECRE\n"
    "67  CAVE\n"
    "68  TURN\n"
    "69  CROSS\n"
    "70  BEDQU\n"
    "1001    KEYS\n"
    "1001    KEY\n"
    "1002    LAMP\n"
    "1002    HEADL\n"
    "1003    GRATE\n"
    "1004    CAGE\n"
    "1005    ROD\n"
    "1006    STEPS\n"
    "1007    BIRD\n"
    "1010    NUGGE\n"
    "1010    GOLD\n"
    "1011    SNAKE\n"
    "1012    FISSU\n"
    "1013    DIAMO\n"
    "1014    SILVE\n"
    "1014    BARS\n"
    "1015    JEWEL\n"
    "1016    COINS\n"
    "1017    DWARV\n"
    "1017    DWARF\n"
    "1018    KNIFE\n"
    "1018    KNIVE\n"
    "1018    ROCK\n"
    "1018    WEAPO\n"
    "1018    BOULD\n"
    "1019    FOOD\n"
    "1019    RATIO\n"
    "1020    WATER\n"
    "1020    BOTTL\n"
    "1021    AXE\n"
    "1022    KNIFE\n"
    "1023    CHEST\n"
    "1023    BOX\n"
    "1023    TREAS\n"
    "2001    TAKE\n"
    "2001    CARRY\n"
    "2001    KEEP\n"
    "2001    PICKU\n"
    "2001    PICK\n"
    "2001    WEAR\n"
    "2001    CATCH\n"
    "2001    STEAL\n"
    "2001    CAPTU\n"
    "2001    FIND\n"
    "2001    WHERE\n"
    "2001    GET\n"
    "2002    RELEA\n"
    "2002    FREE\n"
    "2002    DISCA\n"
    "2002    DROP\n"
    "2002    DUMP\n"
    "2003    DUMMY\n"
    "2004    UNLOC\n"
    "2004    OPEN\n"
    "2004    LIFT\n"
    "2005    NOTHI\n"
    "2005    HOLD\n"
    "2006    LOCK\n"
    "2006    CLOSE\n"
    "2007    LIGHT\n"
    "2007    ON\n"
    "2008    EXTIN\n"
    "2008    OFF\n"
    "2009    STRIK\n"
    "2010    CALM\n"
    "2010    WAVE\n"
    "2010    SHAKE\n"
    "2010    SING\n"
    "2010    CLEAV\n"
    "2011    WALK\n"
    "2011    RUN\n"
    "2011    TRAVE\n"
    "2011    GO\n"
    "2011    PROCE\n"
    "2011    CONTI\n"
    "2011    EXPLO\n"
    "2011    GOTO\n"
    "2011    FOLLO\n"
    "2012    ATTAC\n"
    "2012    KILL\n"
    "2012    STAB\n"
    "2012    FIGHT\n"
    "2012    HIT\n"
    "2013    POUR\n"
    "2014    EAT\n"
    "2015    DRINK\n"
    "2016    RUB\n"
    "3050    OPENS\n"
    "3051    HELP\n"
    "3051    ?\n"
    "3051    WHAT\n"
    "3064    TREE\n"
    "3066    DIG\n"
    "3066    EXCIV\n"
    "3067    BLAST\n"
    "3068    LOST\n"
    "3069    MIST\n"
    "3049    THROW\n"
    "3079    FUCK\n"
    "-1\n"
    "5\n"
    "201  THERE ARE SOME KEYS ON THE GROUND HERE.\n"
    "202  THERE IS A SHINY BRASS LAMP NEARBY.\n"
    "3    THE GRATE IS LOCKED\n"
    "103  THE GRATE IS OPEN.\n"
    "204  THERE IS A SMALL WICKER CAGE DISCARDED NEARBY.\n"
    "205  A THREE FOOT BLACK ROD WITH A RUSTY STAR ON AN END LIES NEARBY\n"
    "206  ROUGH STONE STEPS LEAD DOWN THE PIT.\n"
    "7    A CHEERFUL LITTLE BIRD IS SITTING HERE SINGING.\n"
    "107  THERE IS A LITTLE BIRD IN THE CAGE.\n"
    "8    THE GRATE IS LOCKED\n"
    "108  THE GRATE IS OPEN.\n"
    "209  ROUGH STONE STEPS LEAD UP THE DOME.\n"
    "210  THERE IS A LARGE SPARKLING NUGGET OF GOLD HERE!\n"
    "11   A HUGE GREEN FIERCE SNAKE BARS THE WAY!\n"
    "112  A CRYSTAL BRIDGE NOW SPANS THE FISSURE.\n"
    "213  THERE ARE DIAMONDS HERE!\n"
    "214  THERE ARE BARS OF SILVER HERE!\n"
    "215  THERE IS PRECIOUS JEWELRY HERE!\n"
    "216  THERE ARE MANY COINS HERE!\n"
    "19   THERE IS FOOD HERE.\n"
    "20   THERE IS A BOTTLE OF WATER HERE.\n"
    "120  THERE IS AN EMPTY BOTTLE HERE.\n"
    "221  THERE IS A LITTLE AXE HERE\n"
    "-1\n"
    "6\n"
    "1    SOMEWHERE NEARBY IS COLOSSAL CAVE, WHERE OTHERS HAVE FOUND\n"
    "1    FORTUNES IN TREASURE AND GOLD, THOUGH IT IS RUMORED\n"
    "1    THAT SOME WHO ENTER ARE NEVER SEEN AGAIN. MAGIC IS SAID\n"
    "1    TO WORK IN THE CAVE.  I WILL BE YOUR EYES AND HANDS. DIRECT\n"
    "1    ME WITH COMMANDS OF 1 OR 2 WORDS.\n"
    "1    (ERRORS, SUGGESTIONS, COMPLAINTS TO CROWTHER)\n"
    "1    (IF STUCK TYPE HELP FOR SOME HINTS)\n"
    "2    A LITTLE DWARF WITH A BIG KNIFE BLOCKS YOUR WAY.\n"
    "3    A LITTLE DWARF JUST WALKED AROUND A CORNER,SAW YOU, THREW\n"
    "3    A LITTLE AXE AT YOU WHICH MISSED, CURSED, AND RAN AWAY.\n"
    "4    THERE IS A THREATENING LITTLE DWARF IN THE ROOM WITH YOU!\n"
    "5    ONE SHARP NASTY KNIFE IS THROWN AT YOU!\n"
    "6    HE GETS YOU!\n"
    "7    NONE OF THEM HIT YOU!\n"
    "8    A HOLLOW VOICE SAYS 'PLUGH'\n"
    "9    THERE IS NO WAY TO GO THAT DIRECTION.\n"
    "10   I AM UNSURE HOW YOU ARE FACING. USE COMPASS POINTS OR\n"
    "10   NEARBY OBJECTS.\n"
    "11   I DON'T KNOW IN FROM OUT HERE. USE COMPASS POINTS OR NAME\n"
    "11   SOMETHING IN THE GENERAL DIRECTION YOU WANT TO GO.\n"
    "12   I DON'T KNOW HOW TO APPLY THAT WORD HERE.\n"
    "13   I DON'T UNDERSTAND THAT!\n"
    "14   I ALWAYS UNDERSTAND COMPASS DIRECTIONS, OR YOU CAN NAME\n"
    "14   A NEARBY THING TO HEAD THAT WAY.\n"
    "15   SORRY, BUT I AM NOT ALLOWED TO GIVE MORE DETAIL. I WILL\n"
    "15   REPEAT THE LONG DESCRIPTION OF YOUR LOCATION.\n"
    "16   IT IS NOW PITCH BLACK. IF YOU PROCEED YOU WILL LIKELY\n"
    "16   FALL INTO A PIT.\n"
    "17   IF YOU PREFER, SIMPLY TYPE W RATHER THAN WEST.\n"
    "18   ARE YOU TRYING TO CATCH THE BIRD?\n"
    "19   THE BIRD IS FRIGHTENED RIGHT NOW AND YOU CANNOT CATCH IT\n"
    "19   NO MATTER WHAT YOU TRY. PERHAPS YOU MIGHT TRY LATER.\n"
    "20   ARE YOU TRYING TO ATTACK OR AVOID THE SNAKE?\n"
    "21   YOU CAN'T KILL THE SNAKE, OR DRIVE IT AWAY, OR AVOID IT,\n"
    "21   OR ANYTHING LIKE THAT. THERE IS A WAY TO GET BY, BUT YOU\n"
    "21   DON'T HAVE THE NECESSARY RESOURCES RIGHT NOW.\n"
    "22   MY WORD FOR HITTING SOMETHING WITH THE ROD IS 'STRIKE'.\n"
    "23   YOU FELL INTO A PIT AND BROKE EVERY BONE IN YOUR BODY!\n"
    "24   YOU ARE ALREADY CARRYING IT!\n"
    "25   YOU CAN'T BE SERIOUS!\n"
    "26   THE BIRD WAS UNAFRAID WHEN YOU ENTERED, BUT AS YOU APPROACH\n"
    "26   IT BECOMES DISTURBED AND YOU CANNOT CATCH IT.\n"
    "27   YOU CAN CATCH THE BIRD, BUT YOU CANNOT CARRY IT.\n"
    "28   THERE IS NOTHING HERE WITH A LOCK!\n"
    "29   YOU AREN'T CARRYING IT!\n"
    "30   THE LITTLE BIRD ATTACKS THE GREEN SNAKE, AND IN AN\n"
    "30   ASTOUNDING FLURRY DRIVES THE SNAKE AWAY.\n"
    "31   YOU HAVE NO KEYS!\n"
    "32   IT HAS NO LOCK.\n"
    "33   I DON'T KNOW HOW TO LOCK OR UNLOCK SUCH A THING.\n"
    "34   THE GRATE WAS ALREADY LOCKED.\n"
    "35   THE GRATE IS NOW LOCKED.\n"
    "36   THE GRATE WAS ALREADY UNLOCKED.\n"
    "37   THE GRATE IS NOW UNLOCKED.\n"
    "38   YOU HAVE NO SOURCE OF LIGHT.\n"
    "39   YOUR LAMP IS NOW ON.\n"
    "40   YOUR LAMP IS NOW OFF.\n"
    "41   STRIKE WHAT?\n"
    "42   NOTHING HAPPENS.\n"
    "43   WHERE?\n"
    "44   THERE IS NOTHING HERE TO ATTACK.\n"
    "45   THE LITTLE BIRD IS NOW DEAD. ITS BODY DISAPPEARS.\n"
    "46   ATTACKING THE SNAKE BOTH DOESN'T WORK AND IS VERY DANGEROUS.\n"
    "47   YOU KILLED A LITTLE DWARF.\n"
    "48   YOU ATTACK A LITTLE DWARF, BUT HE DODGES OUT OF THE WAY.\n"
    "49   I HAVE TROUBLE WITH THE WORD 'THROW' BECAUSE YOU CAN THROW\n"
    "49   A THING OR THROW AT A THING. PLEASE USE DROP OR ATTACK INSTEAD.\n"
    "50   GOOD TRY, BUT THAT IS AN OLD WORN-OUT MAGIC WORD.\n"
    "51   I KNOW OF PLACES, ACTIONS, AND THINGS. MOST OF MY VOCABULARY\n"
    "51   DESCRIBES PLACES AND IS USED TO MOVE YOU THERE. TO MOVE TRY\n"
    "51   WORDS LIKE FOREST, BUILDING, DOWNSTREAM, ENTER, EAST, WEST\n"
    "51   NORTH, SOUTH, UP, OR DOWN.  I KNOW ABOUT A FEW SPECIAL OBJECTS,\n"
    "51   LIKE A BLACK ROD HIDDEN IN THE CAVE. THESE OBJECTS CAN BE\n"
    "51   MANIPULATED USING ONE OF THE ACTION WORDS THAT I KNOW. USUALLY \n"
    "51   YOU WILL NEED TO GIVE BOTH THE OBJECT AND ACTION WORDS\n"
    "51   (IN EITHER ORDER), BUT SOMETIMES I CAN INFER THE OBJECT FROM\n"
    "51   THE VERB ALONE. THE OBJECTS HAVE SIDE EFFECTS - FOR\n"
    "51   INSTANCE, THE ROD SCARES THE BIRD.\n"
    "51   USUALLY PEOPLE HAVING TROUBLE MOVING JUST NEED TO TRY A FEW\n"
    "51   MORE WORDS. USUALLY PEOPLE TRYING TO MANIPULATE AN\n"
    "51   OBJECT ARE ATTEMPTING SOMETHING BEYOND THEIR (OR MY!)\n"
    "51   CAPABILITIES AND SHOULD TRY A COMPLETELY DIFFERENT TACK.\n"
    "51   TO SPEED THE GAME YOU CAN SOMETIMES MOVE LONG DISTANCES\n"
    "51   WITH A SINGLE WORD. FOR EXAMPLE, 'BUILDING' USUALLY GETS\n"
    "51   YOU TO THE BUILDING FROM ANYWHERE ABOVE GROUND EXCEPT WHEN\n"
    "51   LOST IN THE FOREST. ALSO, NOTE THAT CAVE PASSAGES TURN A\n"
    "51   LOT, AND THAT LEAVING A ROOM TO THE NORTH DOES NOT GUARANTEE\n"
    "51   ENTERING THE NEXT FROM THE SOUTH. GOOD LUCK!\n"
    "52   IT MISSES!\n"
    "53   IT GETS YOU!\n"
    "54   OK\n"
    "55   YOU CAN'T UNLOCK THE KEYS.\n"
    "56   YOU HAVE CRAWLED AROUND IN SOME LITTLE HOLES AND WOUND UP\n"
    "56   BACK IN THE MAIN PASSAGE.\n"
    "57   I DON'T KNOW WHERE THE CAVE IS, BUT HEREABOUTS NO STREAM\n"
    "57   CAN RUN ON THE SURFACE FOR LONG. I WOULD TRY THE STREAM.\n"
    "58   I NEED MORE DETAILED INSTRUCTIONS TO DO THAT.\n"
    "59   I CAN ONLY TELL YOU WHAT YOU SEE AS YOU MOVE ABOUT\n"
    "59   AND MANIPULATE THINGS. I CANNOT TELL YOU WHERE REMOTE THINGS\n"
    "59   ARE.\n"
    "60   I DON'T KNOW THAT WORD.\n"
    "61   WHAT?\n"
    "62   ARE YOU TRYING TO GET INTO THE CAVE?\n"
    "63   THE GRATE IS VERY SOLID AND HAS A HARDENED STEEL LOCK. YOU\n"
    "63   CANNOT ENTER WITHOUT A KEY, AND THERE ARE NO KEYS NEARBY.\n"
    "63   I WOULD RECOMMEND LOOKING ELSEWHERE FOR THE KEYS.\n"
    "64   THE TREES OF THE FOREST ARE LARGE HARDWOOD OAK AND MAPLE,\n"
    "64   WITH AN OCCASIONAL GROVE OF PINE OR SPRUCE. THERE IS QUITE\n"
    "64   A BIT OF UNDERGROWTH, LARGELY BIRCH AND ASH SAPLINGS PLUS\n"
    "64   NONDESCRITPT BUSHES OF VARIOUS SORTS. THIS TIME OF YEAR\n"
    "64   VISIBILITY IS QUITE RESTRICTED BY ALL THE LEAVES, BUT TRAVEL\n"
    "64   IS QUITE EASY IF YOU DETOUR AROUND THE SPRUCE AND BERRY BUSHES.\n"
    "65   WELCOME TO ADVENTURE!!  WOULD YOU LIKE INSTRUCTIONS?\n"
    "66   DIGGING WITHOUT A SHOVEL IS QUITE IMPRACTICAL: EVEN WITH A\n"
    "66   SHOVEL PROGRESS IS UNLIKELY.\n"
    "67   BLASTING REQUIRES DYNAMITE.\n"
    "68   I'M AS CONFUSED AS YOU ARE.\n"
    "69   MIST IS A WHITE VAPOR, USUALLY WATER, SEEN FROM TIME TO TIME\n"
    "69   IN CAVERNS. IT CAN BE FOUND ANYWHERE BUT IS FREQUENTLY A SIGN\n"
    "69   OF A DEEP PIT LEADING DOWN TO WATER.\n"
    "70   YOUR FEET ARE NOW WET.\n"
    "71   THERE IS NOTHING HERE TO EAT.\n"
    "72   EATEN!\n"
    "73   THERE IS NO DRINKABLE WATER HERE.\n"
    "74   THE BOTTLE OF WATER IS NOW EMPTY.\n"
    "75   RUBBING THE ELECTRIC LAMP IS NOT PARTICULARLY REWARDING.\n"
    "75   ANYWAY, NOTHING EXCITING HAPPENS.\n"
    "76   PECULIAR.  NOTHING UNEXPECTED HAPPENS.\n"
    "77   YOUR BOTTLE IS EMPTY AND THE GROUND IS WET.\n"
    "78   YOU CAN'T POUR THAT.\n"
    "79   WATCH IT!\n"
    "80   WHICH WAY?\n"
    "-1\n"
    "0\n"
};



DEF_TEST_FUNC(adventure)
{
    struct done : public std::runtime_error {
        done() : std::runtime_error("done") {}
    };


    class advent_io_test_stream : public scaffolding::advent_io {
    public:
        advent_io_test_stream(
            const std::vector<std::tuple<std::string, int, double>> & texts,
            bool show_test_output)
        : texts_(texts), show_test_output_(show_test_output) {}

        ~advent_io_test_stream() {}

        std::string getline() override
        {
            TEST_EQUAL(expected_location_.empty(), true);
            TEST_EQUAL(random_value_.empty(), true);

            auto [command, expected_location, random_value] = texts_.at(index_);

            if (show_test_output_)
                std::cout << "\n>" << command << "\n\n";

            std::string buf;

            if (command == "<stop>")
                throw done();
            else if (command == "<console>")
                std::getline(std::cin, buf);
            else {
                buf = command;
                do {
                    if (expected_location != 0)
                        expected_location_.push_back(expected_location);
                    if (random_value >= 0)
                        random_value_.push_back(random_value);
                    ++index_;
                    std::tie (command, expected_location, random_value) = texts_.at(index_);
                } while (command.empty());
            }

            return buf;
        }


        void type(const std::string & msg) override
        {
            if (show_test_output_)
                std::cout << msg;
        }

        void type(int n) override
        {
            if (show_test_output_)
                std::cout << n;
        }

        void trace_location(int loc) override
        {
            if (show_test_output_)
                std::cout << "<" << loc << ">\n";

            TEST_EQUAL(expected_location_.empty(), false);
            if (!expected_location_.empty()) {
                TEST_EQUAL(loc, expected_location_.front());
                expected_location_.pop_front();
            }
        }

        double ran(int call_site) override
        {
            if (show_test_output_)
                std::cout << "ran(" << call_site << ")\n";

            TEST_EQUAL(random_value_.empty(), false);
            if (random_value_.empty())
                return 0.5;

            const double random_value = random_value_.front();
            random_value_.pop_front();
            return random_value;
        }


    private:
        const std::vector<std::tuple<std::string, int, double>> & texts_;
        unsigned index_ = 0;
        std::deque<int> expected_location_;
        std::deque<double> random_value_;
        bool show_test_output_ = true;
    };


    constexpr bool show_test_output = false;

    try {
        // Don't go south from the Swiss cheese room!
        const std::vector<std::tuple<std::string, int, double>> swiss_cheese_bug = {
            {"g",            0,     -1.0},
            {"no",           1,     -1.0},
            {"in",           3,     -1.0},
            {"get lamp",     0,     -1.0},
            {"xyzzy",       11,     -1.0},
            {"light lamp",   0,     -1.0},
            {"low",         10,     -1.0},
            {"get cage",     0,     -1.0},
            {"pit",         14,     -1.0},
            {"east",        13,     -1.0},
            {"get bird",     0,     -1.0},
            {"pit",         14,     -1.0},
            {"down",        15,     -1.0},
            {"stair",       19,      0.1},
            {"drop bird",    0,     -1.0},
            {"north",       28,      0.1},
            {"hole",        36,      0.1},
            {"west",        39,      0.1},
            {"bedquilt",    65,      0.1},
            {"west",        66,      0.1},
            {"south",       77,      0.1}, // -> out-of-range exception; see L39 comment
            {"",             0,      0.1},
            {"<stop>",       0,     -1.0},
        };
        advent_io_test_stream io(swiss_cheese_bug, show_test_output);
        std::istringstream iss(advdat_77_03_31);
        adventure<std::istringstream>(iss, io);
    }
    catch (const done &) {}


    try {
        // Don't go into the pit holding gold!
        const std::vector<std::tuple<std::string, int, double>> infinite_loop_bug = {
            {"g",            0,     -1.0},
            {"no",           1,     -1.0},
            {"in",           3,     -1.0},
            {"get lamp",     0,     -1.0},
            {"xyzzy",       11,     -1.0},
            {"light lamp",   0,     -1.0},
            {"pit",         14,     -1.0},
            {"down",        15,     -1.0},
            {"south",       18,      0.1},
            {"get gold",     0,     -1.0},
            {"hall",        15,      0.1},
            {"y2",          34,      0.1},
            {"down",        33,      0.1},
            {"",             0,      0.1},
            {"plugh",        3,      0.1},
            {"xyzzy",       11,      0.1},
            {"pit",         14,      0.1},
            {"down",        20,      0.1},  // -> "YOU ARE AT THE BOTTOM OF THE PIT WITH A BROKEN NECK.",
            {"",            26,     -1.0},  //    followed by "I DON'T UNDERSTAND THAT!" recurring; see L25 comment
            {"<stop>",       0,     -1.0},
        };
        advent_io_test_stream io(infinite_loop_bug, show_test_output);
        std::istringstream iss(advdat_77_03_31);
        adventure<std::istringstream>(iss, io);
    }
    catch (const done &) {}


    try {
        // How many locations can we visit?
        const std::vector<std::tuple<std::string, int, double>> walkabout = {
            {"g",            0,     -1.0},
            {"no",           1,     -1.0},
            {"west",         2,     -1.0},
            {"east",         1,     -1.0},
            {"in",           3,     -1.0},
            {"get lamp",     0,     -1.0},
            {"get key",      0,     -1.0},
            {"out",          1,     -1.0},
            {"south",        4,     -1.0},
            {"east",         5,     -1.0},
            {"north",        6,      0.4},
            {"valley",       4,     -1.0},
            {"south",        7,     -1.0},
            {"slit",        24,     -1.0},
            {"",             7,     -1.0},
            {"down",         8,     -1.0},
            {"down",        23,     -1.0},
            {"",             8,     -1.0},
            {"unlock grate", 0,     -1.0},
            {"down",         9,     -1.0},
            {"crawl",       10,     -1.0},
            {"light lamp",   0,     -1.0},
            {"get cage",     0,     -1.0},
            {"debris",      11,     -1.0},
            {"get rod",      0,     -1.0},
            {"canyon",      12,     -1.0},
            {"up",          13,     -1.0},
            {"drop rod",     0,     -1.0},
            {"get bird",     0,     -1.0},
            {"get rod",      0,     -1.0},
            {"pit",         14,     -1.0},
            {"crack",       16,     -1.0},
            {"",            14,     -1.0},
            {"down",        15,     -1.0},
            {"hall",        17,      0.1},
            {"jump",        31,      0.1},
            {"",            17,      0.1},
            {"strike fissure", 0,   -1.0}, // create the crystal bridge
            {"jump",        27,      0.1},
            {"north",       40,      0.1},
            {"north",       41,      0.1},
            {"crawl",       60,      0.1},
            {"west",        61,      0.1},
            {"exit",        60,      0.1},
            {"down",        62,      0.1},
            {"north",       63,      0.1},
            {"exit",        62,      0.1},
            {"west",        60,      0.1},
            {"up",          41,      0.1},

            {"climb",       42,      0.1}, // enter the maze of twisty little passages
            {"east",        43,      0.1},
            {"east",        45,      0.1},
            {"east",        46,      0.1},
            {"exit",        45,      0.1},
            {"south",       47,      0.1},
            {"exit",        45,      0.1},
            {"north",       43,      0.1},
            {"south",       44,      0.1},
            {"down",        48,      0.1},
            {"exit",        44,      0.1},
            {"south",       50,      0.1},
            {"up",          49,      0.1},
            {"west",        51,      0.1},
            {"east",        52,      0.1},
            {"up",          53,      0.1},
            {"south",       54,      0.1},
            {"exit",        53,      0.1},
            {"north",       52,      0.1},
            {"east",        55,      0.1},
            {"down",        56,      0.1},
            {"exit",        55,      0.1},
            {"east",        57,      0.1},
            {"south",       58,      0.1},
            {"exit",        57,      0.1},

            {"down",        13,      0.1},
            {"pit",         14,      0.1},
            {"down",        15,      0.1},
            {"south",       18,      0.1},
            {"get gold",     0,     -1.0},
            {"hall",        15,      0.1},
            {"hall",        17,      0.1},
            {"jump",        27,      0.1},
            {"west",        41,      0.1},
            {"north",       59,      0.1},
            {"north",       27,      0.1},
            {"hall",        17,      0.1},
            {"hall",        15,      0.1},
            {"down",        19,      0.1},
            {"west",        32,      0.1},
            {"",            19,      0.1},
            {"drop bird",    0,     -1.0}, // kill the snake
            {"drop rod",     0,     -1.0},
            {"get bird",     0,     -1.0},
            {"get rod",      0,     -1.0},
            {"up",          15,      0.1},
            {"up",          22,      0.1},
            {"",            15,      0.1},
            {"down",        19,      0.1},
            {"north",       28,      0.1},
            {"out",         19,      0.1},
            {"south",       29,      0.1},
            {"out",         19,      0.1},
            {"west",        30,      0.1},
            {"out",         19,      0.1},
            {"north",       28,      0.1},
            {"north",       33,      0.1},
            {"",             0,      0.1},
            {"east",        34,      0.1},
            {"down",        33,      0.1},
            {"",             0,      0.1},
            {"west",        35,      0.1},
            {"y2",          33,      0.1},
            {"",             0,      0.1},
            {"south",       28,      0.1},
            {"hole",        36,      0.1},
            {"crawl",       37,      0.1},
            {"down",        38,      0.1},
            {"climb",       37,      0.1},
            {"crawl",       36,      0.1},
            {"west",        39,      0.1},

            {"bedquilt",    65,      0.1},
            {"east",        64,      0.1},
            {"west",        65,      0.1},
            {"west",        66,      0.1},
            {"west",        67,      0.1},
            {"ne",          72,      0.1},
            {"north",       73,      0.1},
            {"south",       72,      0.1},
            {"sw",          67,      0.1},
            {"east",        66,      0.1},
            {"ne",          65,      0.1},
            {"slab",        68,      0.1},
            {"up",          69,      0.1},
            {"south",       74,      0.1},
            {"down",        75,      0.1},
            {"south",       76,      0.1},
            {"north",       75,      0.1},
            {"north",       77,      0.1},
            {"west",        78,      0.1}, // that's the last room

            {"south",       77,      0.1}, // let's get out of here
            {"north",       66,      0.1},
            {"ne",          65,      0.1},
            {"up",          39,      0.1},
            {"",             0,      0.1},
            {"",             0,      0.1},
            {"east",        36,      0.1},
            {"hole",        28,      0.1},
            {"north",       33,      0.1},
            {"",             0,      0.1},
            {"plugh",        3,      0.1},
            {"out",          1,      0.1},

            {"<stop>",       0,     -1.0},
        };
        advent_io_test_stream io(walkabout, show_test_output);
        std::istringstream iss(advdat_77_03_31);
        adventure<std::istringstream>(iss, io);
    }
    catch (const done &) {}
}

} //namespace Crowther





int main()
{
    std::cout
        << "-----------------------------------------------------------------\n"
        << "     Will Crowther's original 1976 \"Colossal Cave Adventure\"\n"
        << "               A faithful reimplementation in C++\n"
        << "         by Anthony Hay, 2024  (CC0 1.0) Public Domain\n"
        << "              https://github.com/anthay/Adventure\n"
        << "-----------------------------------------------------------------\n"
        << "To quit hit Ctrl-C\n\n";

    try {
        RUN_TESTS();


        class advent_io_console : public scaffolding::advent_io {
        public:
            advent_io_console() {}
            virtual ~advent_io_console() {}

            std::string getline() override
            {
                std::string buf;
                std::getline(std::cin, buf);
                return buf;
            }

            void type(const std::string & msg) override { std::cout << msg; }
            void type(int n) override { std::cout << n; }
        };


        std::istringstream iss(Crowther::advdat_77_03_31);
        advent_io_console io;
        Crowther::adventure<std::istringstream>(iss, io);
    }
    catch (const scaffolding::adventure_pause_exception &) {
        std::cout << "EXECUTION TERMINATED.\n";
        return EXIT_FAILURE;
    }
    catch (const scaffolding::adventure_exception & e) {
        std::cerr << "exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
