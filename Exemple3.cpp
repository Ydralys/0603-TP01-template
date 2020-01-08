/// library: bitstream / exemple 3 : utilisation de BitFloat (codage binaire nombres fractionnaires)
/// author: pascal mignot (université de Reims)
/// version 1.2: mise-à-jour 01/2018
/// + passage à un fonctionnement en flux avec << et >> pour les lectures/ecritures dans le flux
/// + Bits::Block<x> : Block avec x bits valides fixés
/// + Bits::varBlock : Block avec un nombre de bits valides variable
/// + ajout d'une fonction Binary pour visualiser les données en binaires dans un flux.
/// + ajout de tests unitaires pour validation
#include <iostream>
#include <iomanip>
#include <limits>
#include "BitFloat.h"
using namespace std;

int main() {
	cout << "Epsilon = " << std::numeric_limits<double>().epsilon() << endl;
	cout << "sizeof(uint64_t) = " << sizeof(uint64_t) * 8 << endl;
	Bits::Float   b1(0.32456778);
	cout << b1 << " " << setprecision(16) << b1.get() << endl;

	Bits::Float   b2;
	b2.push(1);			cout << b2 << " " << b2.get() << endl;
	b2.push(0);			cout << b2 << " " << b2.get() << endl;
	b2.push(1);			cout << b2 << " " << b2.get() << endl;
	b2.push(1);			cout << b2 << " " << b2.get() << endl;
	b2.push(0);			cout << b2 << " " << b2.get() << endl;
	b2.push(1);			cout << b2 << " " << b2.get() << endl;
	cout << "reel(start=2,len=3) = " << b2.get(2,3) << endl;
	b2.shift(2);
	cout << "shift 2 = " << b2 << " " << b2.get() << endl;
}
