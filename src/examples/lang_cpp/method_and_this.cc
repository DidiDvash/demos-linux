/*
 * This file is part of the linuxapi package.
 * Copyright (C) 2011-2014 Mark Veltzer <mark.veltzer@gmail.com>
 *
 * linuxapi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * linuxapi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with linuxapi. If not, see <http://www.gnu.org/licenses/>.
 */

#include <firstinclude.h>
#include <iostream>	// for std::cout, std::endl
#include <stdlib.h>	// for EXIT_SUCCESS

/*
 * An example of passing object and method and applying the method
 * on the object...
 * This is sometimes called a functor (object+method). You can find
 * similar approaches in boost's bind function.
 */

class A {
public:
	int val;
	void run() {
		std::cout << val << std::endl;
	}
};

void apply(A& a, void (A::*method)()) {
	(a.*method)();
}

int main(int argc, char** argv, char** envp) {
	A a;
	a.val=7;
	apply(a, &A::run);
	return EXIT_SUCCESS;
}
