#include <string>
#include <iostream>

#include <gm_utils.h>

#include "asserttestexit.h"

using namespace std;

static void show_results(int result, int num_of_newlines) {
    cout << "result: " << result << endl;
    cout << "num_of_newlines: " << num_of_newlines << endl;
}

int main() {

    string s = "this\n contains \t special chars";
    int num_of_newlines;

    int result = not_newlines(s, num_of_newlines);
    show_results(result, num_of_newlines);
    assertEquals(0, result);
    assertEquals(0, num_of_newlines);

    result = not_newlines("\n\nnew lines in front", num_of_newlines);
    show_results(result, num_of_newlines);
    assertEquals(2, result);
    assertEquals(2, num_of_newlines);

    result = not_newlines("\n", num_of_newlines);
    show_results(result, num_of_newlines);
    assertEquals(1, result);
    assertEquals(1, num_of_newlines);

    result = not_newlines("\\n", num_of_newlines);
    show_results(result, num_of_newlines);
    assertEquals(0, result);
    assertEquals(0, num_of_newlines);

    string wrapped;

    wrap_cstr(wrapped, 0, 2, s);
    cout << "wrapped: " << wrapped << endl;

    s = "this\\n contains escaped special chars";
    wrapped = "";
    wrap_cstr(wrapped, 0, 2, s);
    cout << "wrapped: " << wrapped << endl;
    assertEquals(s, wrapped);

    s = "this\n contains escaped special chars";
    wrapped = "";
    wrap_cstr(wrapped, 0, 2, s);
    cout << "wrapped: " << wrapped << endl;
    assertEquals("this\\n   contains escaped special chars", wrapped);

    s
            = "this\\n contains escaped special chars near the end of this long long long long long long long string \\nso let's see";
    wrapped = "";
    wrap_cstr(wrapped, 0, 2, s);
    cout << "wrapped: " << wrapped << endl;
    assertEquals(
            "this\\n contains escaped special chars near the end of this long long long long \\n  long long long string \\nso let's see",
            wrapped);

    return 0;
}
