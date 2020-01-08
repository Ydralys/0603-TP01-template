/// library: bitstream / BitBlock.h (stockage de blocs binaires)
/// author: pascal mignot (université de Reims)
/// version 1.2: mise-à-jour 01/2018
/// + passage à un fonctionnement en flux avec << et >> pour les lectures/ecritures dans le flux
/// + Bits::Block<x> : Block avec x bits valides fixés
/// + Bits::varBlock : Block avec un nombre de bits valides variable
/// + ajout d'une fonction Binary pour visualiser les données en binaires dans un flux.
/// + ajout de tests unitaires pour validation
#ifndef _BITBLOCK
#define _BITBLOCK
#include <algorithm>
#include "BitBase.h"

#undef _DEBUG
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
	// classe technique template SupportType dont le but est de produire le type d'un
	// support sous-jacent en fonction du nombre bits.
	template <int Nbytes> struct BlockTypeImpl;
	template <> struct BlockTypeImpl<1> { typedef uint8_t  Type; };
	template <> struct BlockTypeImpl<2> { typedef uint16_t Type; };
	template <> struct BlockTypeImpl<3> { typedef uint32_t Type; };
	template <> struct BlockTypeImpl<4> { typedef uint64_t Type; };

	/// class Bits::Block
	/// classe permettant de définir un paquet de bits de 1 à 64 bitsde taille fixe
	/// les bits valides doivent sont stockés dans les LSBs.
	template <int NBITS> class Block {
		/// Type sous-jacent utilisé pour stocker les bits du Bits::Block.
	public:	using Type = typename
		BlockTypeImpl<(NBITS >  0) + (NBITS > 8) + (NBITS > 16) + (NBITS > 32)>::Type;
	protected:
		/// support sous-jacent de stockage (typiquement uint8_t, uint16_t, uint32_t, uint64_t)
		/// déduit de NBITS et utilisé pour stocker les bits du bloc.
		Type		bits;
	public:
		/// @name Constructeurs
		//@{
		/// @brief constructeur par défaut : nb bits valides par défaut, valeur à 0
		inline Block() : bits(0) {}
		/// @brief constructeur par valeur. les bits au-delà du support valide sont perdus.
		inline Block(Type bits_to_store) :	bits( bits_to_store & Bits::mask<Type>(0, Size_t(NBITS)) ) {
			DEBUG( if (bits != bits_to_store) WARNING("Perte de précision (la valeur %u nécessite plus de %u bits)",Size_t(bits_to_store),Size_t(NBITS)) );
		}
		//@}

		/// @name utilitaires
		//@{
		/// @brief retourne un mask binaire à 1 sur tous les bits valides et à 0 hors valide.
		inline Type mask() const { return Bits::mask<Type>(0u, Size_t(NBITS)); }
		/// @brief efface tous les bits du bloc (y compris hors valide).
		inline void clear() { bits = 0; }
		/// @brief met à 0 les bits hors valide.
		inline void clean() { bits &= mask(); }
		//@}

		/// @name setters
		//@{
		/// @brief stocke sans modifier le nombre de bits valides (hors valide non nettoyés)
		inline void set(Type bits_to_store) {
			bits = bits_to_store & mask();
            DEBUG( if (bits != bits_to_store) WARNING("Perte de précision (la valeur %u nécessite plus de %u bits)",Size_t(bits_to_store),Size_t(NBITS)) );
		}

		/// @brief fixe la valeur du bit Position à BitValue, mais seulement si Position est dans le support valide.
		/// Donc, il n'est possible de fixe un bit à l'extérieur des bits valides.
		inline void set_bit(Size_t Position, Bit BitValue) {
			if (Position < Size_t(NBITS)) bits = ::Bits::set<Type>(bits, Position, BitValue);
            else { DEBUG(WARNING("Echec (Position = %u plus grand que NBITS = %d)",Position,NBITS)); }
		}
		//@}

		/// @name getters
		//@{

		/// @brief retourne le contenu du Block sous forme dans un entier de la taille du support sous-jacent.
		inline Type get() const { return bits & mask(); };
		/// @brief retourne le contenu brut du Block sous forme dans un entier de la taille du support sous-jacent.
		/// Fonction utilisée dans la validation de la classe.
		inline Type get_raw() const { return bits; };
		/// @brief retourne le nombre de bits valides.
		inline Size_t get_valid() const { return Size_t(NBITS); };
        /// @brief retourne le MSB.
        inline Size_t MSB() const { return std::min<Type>(Bits::MSB(bits),NBITS); };

        //@}


		/// @name opérateurs
		//@{
		/// @brief compare deux Block sur la seule base de leurs bits valides
		/// Ils peuvent donc avoir un nombre de bits valides différents et être
		/// égaux s'ils stockent le même nombre binaire.
		friend bool operator==(const Block<NBITS> &a, const Block<NBITS> &b) {
			return (a.mask() & a.bits) == (b.mask() & b.bits);
		}
		/// @brief retourne le ET logique entre deux Blocks.
		/// Leurs nombres de bit valides peuvent être différents
		friend Block<NBITS> operator^(const Block<NBITS> &a, const Block<NBITS> &b) {
			return Block<NBITS>(a.bits ^ b.bits);
		}
		/// @brief effectue une rotation circulaire à gauche sur le support valide uniquement.
		/// exemple: bits = 00001101, NBITS=4 conduit à bits = 00001011
        inline void RotateLeft(Byte r) {
			r %= Size_t(NBITS);
			if (r) {
				clean();
				bits = ((bits << r) | (bits >> (Size_t(NBITS) - r))) & (this->mask());
			}
		}
		/// @brief effectue une rotation circulaire à droite sur le support valide uniquement.
		/// exemple: bits = 00001101, NBITS=4 conduit à bits = 00001110
        inline void RotateRight(Byte r) {
			r %= Size_t(NBITS);
			if (r) {
				clean();
				bits = ((bits >> r) | (bits << (Size_t(NBITS) - r))) & (this->mask());
			}
		}
		/// @brief opérateur + entre deux Blocks.
		/// L'opération ne fait jamais gagner de précision.
		/// S'il y a une retenue sur le résultat de l'opération, celle-ci est perdue.
		/// Autrement dit, l'addition se fait modulo 2^max(_valid).
		/// Le Block renvoyé a le nombre de bit valide du plus grand des deux.
		friend Block<NBITS> operator+(const Block<NBITS> &x1, const Block<NBITS> &x2) {
			return Block<NBITS>( (x1.bits + x2.bits) & x1.mask() );
		}

		/// @brief opérateur - entre deux Blocks.
		/// L'opération ne fait jamais gagner de précision.
		/// Si le résultat est négatif, on obtient le complément à 1.
		/// Le Block renvoyé a le nombre de bit valide du plus grand des deux.
		friend Block<NBITS> operator-(const Block<NBITS> &x1, const Block<NBITS> &x2) {
			if (x1.bits > x2.bits) {
				return Block<NBITS>((x1.bits - x2.bits) & x1.mask());
			}
			else {
				Type    carry = Type(1) << Size_t(NBITS);
				return Block<NBITS>((carry - x2.bits + x1.bits) & x1.mask());
			}
		}

		/// @brief surcharge pour affichage dans un flux de sortie
		friend std::ostream& operator<<(std::ostream &os, const Block<NBITS> &x) {
			return os << Binary<Type>(x.bits,Size_t(NBITS),8*sizeof(Type)-Size_t(NBITS),Size_t(NBITS));
		}
		//@}

	};

	/// class Bits::VarBlock
	/// classe permettant de définir un paquet de bits de taille variable (64bits max)
	class varBlock {
	public:	using Type = uint64_t;
	protected:
		/// @brief nombre de bits valides
		Size_t  valid;
		/// @brief stockage des bits
		Type	bits;
		static constexpr Size_t maxbits = sizeof(Type)*8;
	public:
		/// @name Constructeurs
		//@{
		/// @brief constructeur par défaut : nb bits valides par défaut, valeur à 0
		inline varBlock() : valid(maxbits), bits(0u) {}
		/// @brief constructeur taille support (valeur non initialisée).
		inline varBlock(Size_t _valid) : valid(_valid) {
			if ( (_valid>=0) && (_valid<=maxbits) ) {
				std::cout << "arg";
			}
			assert( (_valid>=0) && (_valid<=maxbits) );
		}
		/// @brief constructeur taille support + valeur.
		/// les bits au-delà du support valide sont perdus.
		inline varBlock(Size_t _valid, Type bits_to_store) :
			valid(_valid), bits( bits_to_store & Bits::mask<Type>(0u,valid) ) {
			assert( (_valid>=0) && (_valid<=maxbits) );
            DEBUG( if (bits != bits_to_store) WARNING("Perte de précision (la valeur %u nécessite plus de %u bits)",Size_t(bits_to_store),Size_t(valid)) );
		}
		//@}

		/// @name utilitaires
		//@{
		/// @brief retourne un mask binaire à 1 sur tous les bits valides et à 0 hors valide.
		inline Type mask() const { return Bits::mask<Type>(0u,valid); }
		/// @brief efface tous les bits du bloc (y compris hors valide).
		inline void clear() { bits = 0; }
		/// @brief met à 0 les bits hors valide.
		inline void clean() { bits &= mask(); }
		//@}

		/// @name setters
		//@{
		/// @brief stocke sans modifier le nombre de bits valides (hors valide non nettoyés)
		inline void set(Type bits_to_store) {
            bits = bits_to_store & mask();
            DEBUG( if (bits != bits_to_store) WARNING("Perte de précision (la valeur %u nécessite plus de %u bits)",Size_t(bits_to_store),Size_t(valid)) );
        }

		/// @brief fixe la valeur du bit Position à BitValue, mais seulement si Position est dans le support valide.
		/// Donc, il n'est possible de fixe un bit à l'extérieur des bits valides.
		inline void set_bit(Size_t Position, Bit BitValue) {
            assert( Position<maxbits );
			if (Position < valid) bits = ::Bits::set<Type>(bits, Position, BitValue);
            else { DEBUG(WARNING("Echec (Position = %d plus grand que valid = %d)",Position,valid)); }
		}

		/// @brief change le nombre de bits valides.
		/// Si le nombre de bits augmente, il est garanti que les bits gagnés sont des 0.
		/// Si le nombre de bits diminue, les bits perdus à l'extérieur du support sont mis à zéro.
		inline void set_valid(Size_t _valid) {
            assert( (_valid>=1) && (_valid<=maxbits) );       // au moins 1 bit
			if (_valid > valid) { clean();  valid = _valid; }
			else if (_valid < valid) { valid=_valid; clean(); }
		}
		//@}

		/// @name getters
		//@{

		/// @brief retourne le contenu du Block sous forme dans un entier de la taille du support sous-jacent.
		inline Type get() const { return bits & mask(); };
		/// @brief retourne le contenu brut du Block sous forme dans un entier de la taille du support sous-jacent.
		/// Fonction utilisée dans la validation de la classe.
		inline Type get_raw() const { return bits; };
		/// @brief retourne le nombre de bits valides
		inline Size_t get_valid() const { return valid; };
        /// @brief retourne le bit souhaité
        inline Bit get_bit(Size_t Position) const {
            assert( Position<valid );
            return ::Bits::get<Type>(bits,Position);
        };
        /// @brief test si le contenu est zéro
        inline bool is_zero() const {
            return ! (bits & mask());
        }
        /// @brief retourne le double associé au nombre binaire
        inline double get_double() const {
            Type    v = bits;
            Size_t  n = valid;
            double  result = 0.0, pow2 = 0.5;
            for(Size_t i=0;i<n;++i) {
                result /= 2.0;
                if (v & 0x1) result += pow2;
                v >>= 1;
            }
            return result;
        }
        /// @brief retourne le double associé au nombre binaire
        inline double get_double(unsigned int p, unsigned int q) const {
            double  result = 0.0, pow2 = 0.5;
            for(Size_t i=0;i<q;++i) {
                // std::cout << p+i << ":" << (x.get_bit(valid-(p+i+1)) ? "1" : "0") << " ";
                if (get_bit(valid-(p+i+1))) result += pow2;
                pow2 /= 2.0;
            }
            // std::cout << std::endl;
            return result;
        }




        //@}


		/// @name opérateurs
		//@{
		/// @brief compare l'égalité de deux Block variable sur la seule base de leurs bits valides
		/// Ils peuvent donc avoir un nombre de bits valides différents et être
		/// égaux s'ils stockent le même nombre binaire.
		friend bool operator==(const varBlock &a, const varBlock &b) {
			return (a.mask() & a.bits) == (b.mask() & b.bits);
		}

        /// @brief compare deux Block variable sur la seule base de leurs bits valides
        /// Ils peuvent donc avoir un nombre de bits valides différents et être
        /// égaux s'ils stockent le même nombre binaire.
        friend bool operator<(const varBlock &a, const varBlock &b) {
            return (a.mask() & a.bits) < (b.mask() & b.bits);
        }

        /// @brief compare exactement deux Block variable (valeur et support)
        /// Retourne vrai s'ils n'ont pas le même nombre de bits valides ou
        /// ont une valeur différente.
        /// Pour la valeur, seuls les bits valides sont comparés.
        friend bool operator!=(const varBlock &a, const varBlock &b) {
            return (a.get_valid() != b.get_valid())
                    || ((a.mask() & a.bits) != (b.mask() & b.bits));
        }

		/// @brief retourne le ET logique entre deux Blocks.
		/// Leurs nombres de bit valides peuvent être différents
		friend varBlock operator^(const varBlock &a, const varBlock &b) {
			return {std::max(a.valid, b.valid), a.bits ^ b.bits};
        }
		/// @brief effectue une rotation circulaire à gauche sur le support valide uniquement.
		/// exemple: bits = 00001101, NBITS=4 conduit à bits = 00001011
        inline void RotateLeft(Size_t r) {
			r %= valid;
			if (r) {
				clean();
				bits = ((bits << r) | (bits >> (valid - r))) & mask();
			}
		}
		/// @brief effectue une rotation circulaire à droite sur le support valide uniquement.
		/// exemple: bits = 00001101, NBITS=4 conduit à bits = 00001110
        inline void RotateRight(Size_t r) {
			r %= valid;
			if (r) {
				clean();
				bits = ((bits >> r) | (bits << (valid - r))) & mask();
			}
		}

        /// @brief décalage à droite (avec diminution du nombre de bits valides)
        /// exemple: bits = 00001101, NBITS=2 conduit à bits = 000011
        inline void ShiftRight(Size_t r) {
            r = std::min(valid,r);
            if (r) {
                clean();
                bits >>= r;
                valid -= r;
            }
        }

        /// @brief décalage à gauche (avec augmentation du nombre de bits balides
        /// exemple: bits = 00001101, NBITS=2 conduit à bits = 0000110100
        inline void ShiftLeft(Size_t r) {
            if (r) {
                clean();
                bits <<= r;
                valid = std::min(valid+r,maxbits);
            }
        }

        /// @brief décalage à gauche (avec augmentation du nombre de bits balides
        /// exemple: bits = 00001101, NBITS=2 conduit à bits = 0000110100
        inline void ShiftLeftAndAdd(const Bit &b) {
            bits <<= 1;
            bits |= b & 0x1;
            valid = std::min(valid+1,maxbits);
        }

		/// @brief opérateur + entre deux Blocks.
		/// L'opération ne fait jamais gagner de précision.
		/// S'il y a une retenue sur le résultat de l'opération, celle-ci est perdue.
		/// Autrement dit, l'addition se fait modulo 2^max(_valid).
		/// Le Block renvoyé a le nombre de bit valide du plus grand des deux.
		friend varBlock operator+(const varBlock &x1, const varBlock &x2) {
			const varBlock &x = (x1.valid > x2.valid ? x1 : x2);
			return {x.valid, (x1.bits + x2.bits) & x.mask() };
        }

		/// @brief opérateur - entre deux Blocks.
		/// L'opération ne fait jamais gagner de précision.
		/// Si le résultat est négatif, on obtient le complément à 1.
		/// Le Block renvoyé a le nombre de bit valide du plus grand des deux.
		friend varBlock operator-(const varBlock &x1, const varBlock &x2) {
			const varBlock &x = (x1.valid > x2.valid ? x1 : x2);
			if (x1.bits > x2.bits) {
				return { x.valid, (x1.bits - x2.bits) & x.mask() };
			}
			else {
				Type   carry = Type(1) << x.valid;
				return { x.valid, (carry - x2.bits + x1.bits) & x.mask() };
			}
		}

		/// @brief surcharge pour affichage dans un flux de sortie
		friend std::ostream& operator<<(std::ostream &os, const varBlock &x) {
			return os << Binary<Type>(x.bits,x.valid,8*Size_t(sizeof(Type))-x.valid,x.valid);
		}
		//@}
	};
}

#undef WARNING
#undef DEBUG
#endif
