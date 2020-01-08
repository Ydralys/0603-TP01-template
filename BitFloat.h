/// library: bitstream / complement (codage binaire nombres fractionnaires)
/// author: pascal mignot (université de Reims)
/// version 1.2: mise-à-jour 01/2018
/// + passage à un fonctionnement en flux avec << et >> pour les lectures/ecritures dans le flux
/// + Bits::Block<x> : Block avec x bits valides fixés
/// + Bits::varBlock : Block avec un nombre de bits valides variable
/// + ajout d'une fonction Binary pour visualiser les données en binaires dans un flux.
/// + ajout de tests unitaires pour validation

#ifndef _BITFLOAT
#define _BITFLOAT
#include <cstdint>
#include <iostream>
#include "BitBase.h"

namespace Bits {
	class Float {
	private:
		using storage_t = uint64_t;			///< type sous-jacent de stockage
		static const int MaxBits = 64;		///< nombre de bit du type sous-jacent
		static const storage_t Mask = 1ULL << (MaxBits - 1); ///< masque pour extraction du MSB
		storage_t		storage;			///< stokage du flottant en binaire fractionnaire
		uint32_t		valid;				///< nombre de bits valides dans le flottant
	public:
		/// constructeur à partir d'un double: remplit le Bits:Float à partir d'un double
		/// logiquement, le nombre de bits valide à la fin de ce constructeur est 53
		Float(double v) : storage(0), valid(0) {
			assert((v >= 0.0) && (v < 1.0));
			while (v != 0.0) {
				storage <<= 1;
				v *= 2.0;
				if (v >= 1.0) {
					v -= 1.0;
					storage |= 1;
				}
				++valid;
				if (valid == MaxBits) break;
			}
			storage <<= MaxBits - valid;
		}

		/// retourne le nombre de bits valide dans le mot
		/// Rappel: devrait être en permanence au minimum à qmin (cf poly), ou plus
		/// si q est incrémenté. Push pour en ajouter
		uint32_t getValidBits() const { return valid;  }

		/// constructeur par défaut: construit un flottant binaire fractionnaire égal à 0
		Float() : storage(0), valid(0) {}

		/// shift le nombre binaire à gauche de nshift bit = fait perdre les nshift MSB.
		/// dans l'algorithme, associé au décalage de la position de départ.
		/// réduit le nombre de bits valide du nombre de shift (par défaut 1)
		void shift(uint32_t nshift = 1) {
			storage <<= nshift;
			valid = (valid <= nshift ? 0 : valid - nshift);
		}

		/// ajoute un bit en dernière position du binaire fractionnaire.
		/// augmente le nombre de bits valide de 1.
		bool push(Bit v) {
			assert((v == 0) || (v == 1));
			if (valid == MaxBits) return false;
			++valid;
			storage |= static_cast<storage_t>(v) << (MaxBits - valid);
			return true;
		}

		/// retourne le double associé au binaire fractionnaire complet
		double get() const {
			double    pow2 = 0.5, r=0.0;
			storage_t  v = storage;
			while (v != 0) {
				if (v & Mask) r += pow2;
				v <<= 1;
				pow2 /= 2.0;
			}
			return r;
		}

		/// retourne le double associé au binaire fractionnaire allant des bits first é first+nb-1
		/// bit de poids le plus fort = 1
		double get(int first, int nb) const {
			double    pow2 = 0.5, r = 0.0;
			storage_t  v = storage << (first-1);
			for (int i = 0; i < nb; i++) {
				if (v & Mask) r += pow2;
				v <<= 1;
				pow2 /= 2.0;
			}
			return r;
		}

		/// surcharge d'un Bits::Float dans le flux de sortie
		/// l'affiche sous forme binaire précédé d'un point
		/// le nombre de bits affichs correspond au nombre de bits valides
		friend std::ostream& operator<<(std::ostream& os, const Float &f) {
			storage_t   v = f.storage;
			os << '.';
			for (uint32_t i = 0; i < f.valid; i++) {
				os << (v & f.Mask ? 1 : 0);
				v <<= 1;
			}
			return os;
		}

	};
}

#endif
