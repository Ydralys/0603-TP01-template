/// library: bitstream / BitBase.h (opérations binaires de base)
/// author: pascal mignot (université de Reims)
/// version 1.2: mise-à-jour 01/2018
/// + passage à un fonctionnement en flux avec << et >> pour les lectures/ecritures dans le flux
/// + Bits::Block<x> : Block avec x bits valides fixés
/// + Bits::varBlock : Block avec un nombre de bits valides variable
/// + ajout d'une fonction Binary pour visualiser les données en binaires dans un flux.
/// + ajout de tests unitaires pour validation

#ifndef _BITBASE
#define _BITBASE
#include <cstdint>
#include <type_traits>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <typeinfo>

#ifdef _DEBUG
#include <stdio.h>
#define DEBUG(x) x
#ifndef __PRETTY_FUNCTION__
#define WARNING(x,...) printf("WARNING:%s: " x "\n",__FUNCTION__,__VA_ARGS__)
#else
#define WARNING(x,...) printf("WARNING in %s\n\t" x "\n",__PRETTY_FUNCTION__,__VA_ARGS__)
#endif
#else
#define DEBUG(x)
#endif

namespace Bits {
	using Size_t = unsigned int;
	using Byte = unsigned char;
	using Bit = bool;

	// classe template SupportType dont le but est de produire le type d'un
	// support sous-jacent en fonction du nombre bits.
	template <size_t SIZEOF> struct uTypeImpl;
	template <> struct uTypeImpl<1> { typedef uint8_t  Type; };
	template <> struct uTypeImpl<2> { typedef uint16_t Type; };
	template <> struct uTypeImpl<4> { typedef uint32_t Type; };
	template <> struct uTypeImpl<8> { typedef uint64_t Type; };

	template <typename T> size_t size() {
		return 8*sizeof(T);
	}

    /// @brief récupère le bit à la position pos (pos = 0 est le LSB)
    /// @detail Position doit être strictement intérieur à 8*sizeof(T).
    template <typename T> Bit get(const T &x, const Size_t Position) {
        assert( (Position < 8*Size_t(sizeof(T))) && "Position au-delà du MSB");
        return (Bit)((x >> Position) & T(1u));
    }

    /// @brief classe technique intermédiaire pour affichage en binaire d'objets bruts dans le flux
	/// @detail cette classe est utilisée automatiquement par l'intermédiaire de la fonction Binary(...)
    template <class T> struct BinaryObject {
        const T&	  value;
        const Size_t  maxbit,pack,offset;
        BinaryObject(const T&	_value, const Size_t _pack, const Size_t _offset, const Size_t _maxbit):
                value(_value), maxbit(_maxbit?_maxbit:8*Size_t(sizeof(T))+1), pack(_pack?_pack:maxbit),  offset{_offset} {}
        friend std::ostream& operator<<(std::ostream &stream, const BinaryObject &v) {
			using uType = typename uTypeImpl<sizeof(T)>::Type;
			const uType	&uvalue = *reinterpret_cast<const uType*>(&v.value);
            Size_t 	ibit = 0; // rbit = numéro relatif du bit
			for(Size_t i=8*Size_t(sizeof(T))-1;true;--i) {
				if (ibit >= v.offset) {
					stream << (get<uType>(uvalue, i) ? '1' : '0');
					Size_t rbit = ibit - v.offset + 1;
					if (rbit == v.maxbit) break;
					if (rbit % v.pack == 0) stream << ' ';
				}
				++ibit;
				if (i == 0) break;
			}
            return stream;
        }
    };

	/// @brief fonction à utiliser pour affichage en binaire d'un objet dans le flux.
	/// @param pack groupe les bits par paquets de pack (0 = pas de packing)
	/// @param offset commence à offset bit depuis le début de l'objet. Les bits avant l'offset ne sont pas affiché (0 = pas d'offset).
	/// @param maxbit affiche maxbit au maximum (0 = tous).
	template <class T> BinaryObject<T> Binary(const T& v, const Size_t pack=0, const Size_t offset=0, const Size_t maxbit=0)
		{	return BinaryObject<T>(v,pack,offset,maxbit); }

	/// @brief construction d'un masque de Width bits décalé de Position bits à gauche.
	/// @detail Pour Width=0, le masque est sur les bits [0,Width-1] où 0 est le LSB.
	/// Si Position + Width dépasse 8*sizeof(T), le masque est tronqué au-delà.
	/// Position doit être strictement intérieur à 8*sizeof(T).
	/// Width doit être inférieur ou égal à 8*sizeof(T).
	template <typename T> T mask(const Size_t Position, const Size_t Width) {
		assert( (Position < 8*Size_t(sizeof(T))) && "Position au delà du MSB");
		assert( (Width <= 8*Size_t(sizeof(T)))   && "Width plus grand que le type");
		T  v = T(Width == 8*Size_t(sizeof(T)) ? 0 : T(~T(0)) << Width);
		return static_cast<T>(~(v) << Position);
	};

	/// @brief construction du masque associé au bit i (0 = LSB), à savoir tous les bits à 0 sauf le bit i.
	/// @detail Position doit être strictement intérieur à 8*sizeof(T).
	template <typename T> T bitmask(const Size_t Position) {
		assert( (Position < 8*Size_t(sizeof(T))) && "Position au delà du MSB" );
		return static_cast<T>(1) << Position;
	}

	/// @brief retourne x dans lequel la valeur du bit à la position Position est fixé à la valeur Value (0=LSB).
	/// @detail Position doit être strictement intérieur à 8*sizeof(T).
	template <typename T> T set(T x, Size_t Position, Bit Value) {
		assert( (Position < 8*Size_t(sizeof(T))) && "Position au delà du MSB");
		T 	Mask = bitmask<T>(Position);
		T   Result = (Value ? ~static_cast<T>(0) : static_cast<T>(0));
		return (x & ~Mask) | (Result & Mask);
	}

	/// @brief Retourne x en remplaçant ses bits de [Position,Position+Width-1] par les bits [0,Width-1] de y.
	/// @detail Position doit être strictement intérieur à 8*sizeof(T).
	/// Width doit être inférieur ou égal à 8*sizeof(T).
	/// Si Position + Width dépasse 8*sizeof(T), la copie est tronquée au-delà.
	/// Ni x ni y ne sont modifiés par cette fonction.
	template <typename T> T set(const T &x, const Size_t Position, const Size_t Width, const T &y) {
		assert( (Position < 8*Size_t(sizeof(T))) && "Position au delà du MSB");
		assert( (Width <= 8*Size_t(sizeof(T)))   && "Width plus grand que le type");
		return static_cast<T>(
			  ( x              & ( ~mask<T>(Position, Width) ))
			| ((y << Position) & (  mask<T>(Position, Width) ))
		);
	}

	/// @brief calcule le bit de poids le plus fort de x (i.e. le bit d'indice le plus grand différent de 0)
	/// @detail Exemples: retourne 1 si x=0001, 2 si x=0011, 3 si x=0110, 4 si x=1001, etc ...
	/// Si x=0, alors la fonction retourne 0.
	template <typename T> Size_t MSB(T x) {
		Size_t k = 0;
		while (x) {
            x = T(x >> 1);
            ++k;
        }
		return k;
	}

    template <class T> T RotateLeft(T bits, int rot) {
        return (T(bits << rot) | T(bits >> T( 8*sizeof(T) - T(rot))));
    }
    template <class T> T RotateRight(T bits, int rot) {
        return (T(bits >> rot) | T(bits << T( 8*sizeof(T) - T(rot))));
    }


}

#undef WARNING
#undef DEBUG
#endif
