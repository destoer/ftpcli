#include <iostream>

int main() {
	int code = 220;
	bool x = !((code >= 200) && (code < 300));
	std::cout << x;
}