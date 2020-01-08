/// library: bitstream / exemple 2 (exemple écriture/lecture depuis un Bits::Stream)
/// author: pascal mignot (université de Reims)
/// version 1.2: mise-à-jour 01/2018
/// + passage à un fonctionnement en flux avec << et >> pour les lectures/ecritures dans le flux
/// + Bits::Block<x> : Block avec x bits valides fixés
/// + Bits::varBlock : Block avec un nombre de bits valides variable
/// + ajout d'une fonction Binary pour visualiser les données en binaires dans un flux.
/// + ajout de tests unitaires pour validation

#include <iostream>
#include <fstream>
#include <random>
#include <vector>

// vous devez lire l'implémentation de bitstream avant de l'utiliser
#include "BitStream.h"
using namespace std;

int main() {
	// initialisation nombres aléatoires
	random_device rd;  // seed
	mt19937 gen(rd()); // mersenne_twister_engine
	uniform_int_distribution<> bin(0,31);  // v.a. de 0 à 31

	// remplissage d'un vecteur avec de v.a. de 0 à 31
	const int NbValues = 23;
	vector<int>  v;
	v.reserve(NbValues);
	for(int i=0;i<NbValues;++i) v.push_back( bin(gen) );

	// affichage des valeurs dans le vecteur
	cout << "Valeurs du vecteur: ";
	for(int x : v) cout << x << " ";
	cout << endl;

	// codage dans des blocks de 5 bits
	cout << "Copie dans un vecteur de Bit::Block<5>" << endl;
	vector<Bits::Block<5>>    vIn,vOut;
	using StorageType = Bits::Block<5>::Type; // = uint8_t, uint16_t, uint32_t ou uint64_t suivant le nb de bits
	// note: cast pour éviter les warnings int -> uintX_t avec X=8 ou 16.
	for(int x : v) vIn.push_back( StorageType(x) );
	cout << "Valeurs du vecteur: ";
	for(auto x : vIn) cout << x << " ";
	cout << endl;

	// écriture de ces nValues dans le stream (avec bitlen bits par valeur)
	cout << "Creation du Bit::Stream 1" << endl;
	Bits::Stream			stream1;
	cout << "Ecriture du vecteur Bit::Block<5> dans le stream" << endl;
	for(auto x : vIn) stream1 << x;
	cout << "Taille stockée dans le stream = 5 x "
		<< vIn.size() << " = " << stream1.get_bit_size() << " bits" << endl;

	// affichage direct du contenu du stream
	cout << "Contenu de stream1: " << stream1 << endl;
	cout << "Contenu de stream1 (par paquets de 5): " << Binary(stream1,5) << endl;
	
	// lecture des données écrites dans le stream
	stream1.seek(0);		// place le pointeru de lecture au début du stream
	while( ! stream1.end_of_stream() ) {
		Bits::Block<5>	b;
		stream1 >> b;
		vOut.push_back(b);
	}
	cout << "Valeurs du vecteur relues: ";
	for(auto x : vOut) cout << x << " ";
	cout << endl;

	// écriture des valeurs du stream dans un fichiers
	const char *OutputFile = "data.bin";
	ofstream  file1(OutputFile, std::ios::out | std::ios::binary );
	file1.write(stream1.get_buffer(), stream1.get_byte_size());
	file1.close();
	cout << "La taille du fichier devrait être de " << stream1.get_byte_size() << " octets." << endl;

	// tailles des données stockées dans le fichier (pour le rechargement)
	// normalement ces données devraient être dans l'entête du fichier
	const Bits::Size_t
		nBytes = stream1.get_byte_size(),
		nBits  = stream1.get_bit_size();

	// on recharge les données dans un autre stream
	cout << "Creation du Bit::Stream 2" << endl;
	Bits::Stream		stream2;
	// allocation mémoire pour les données du flux
	stream2.request_storage_size(nBytes);
	// lecture des données depuis le fichier
	ifstream  file2(OutputFile, std::ios::in | std::ios::binary);
	file2.read(stream2.get_buffer(), nBytes);
	file2.close();
	// fixe manuellement le pointeur d'écriture
	stream2.write_seek(nBits);

	// on relit et on compare les données dans les deux flux
	vector<Bits::Block<5>>		vFile;
	stream2.seek(0);
	while (! stream2.end_of_stream()) {
		Bits::Block<5>	b;
		stream2 >> b;
		vFile.push_back(b);
	}

	// comparaison
	cout << "vIn.size = " << vIn.size() << " / vFile.size = " << vFile.size() << endl;
	cout << "Valeurs du vecteur relues depuis le fichier: " << endl;
	cout << "en binaire: ";
	for(auto x : vFile) cout << x << " ";
	cout << endl;
	cout << "en entier: ";
	for(auto x : vFile) cout << int(x.get()) << " ";
	cout << endl;
	// note que le cas est nécessaire ci-dessus car Bits::Block<5> produit un support
	// uint8_t qui n'est pas interprété comme un entier par

	cin.get();
	return 0;
}
