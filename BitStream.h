/// library: bitstream / BitStream.h (gestion de flux de bits)
/// author: pascal mignot (université de Reims)
/// version 1.2: mise-à-jour 01/2018
/// + passage à un fonctionnement en flux avec << et >> pour les lectures/ecritures dans le flux
/// + Bits::Block<x> : Block avec x bits valides fixés
/// + Bits::varBlock : Block avec un nombre de bits valides variable
/// + ajout d'une fonction Binary pour visualiser les données en binaires dans un flux.
/// + ajout de tests unitaires pour validation
/// 1.2-6 : Stream copy/assignation fix
/// 1.2-7 : Stream::Position refactoring & cleaning


#ifndef _BITSTREAM
#define _BITSTREAM
#include <cstring>
#include "BitBase.h"
#include "BitBlock.h"

#ifdef _DEBUG
#include <stdio.h>
#define DEBUG(x) x
#ifndef __PRETTY_FUNCTION__
#define WARNING(x,...) printf("WARNING:%s: " x "\n",__FUNCTION__,__VA_ARGS__)
#else
#define WARNING(x,...) printf("WARNINGING in %s\n\t" x "\n",__PRETTY_FUNCTION__,__VA_ARGS__)
#endif
#else
#define DEBUG(x)
#endif

namespace Bits {
	/// @brief classe technique intermédiaire pour affichage en binaire d'un tableau d'objets bruts dans le flux
	/// @detail cette classe est utilisée automatiquement par l'intermédiaire de la fonction Binary(...)
	template <class T> struct BinaryArray {
		const T			*array;
		const Size_t    size,pack,offset,maxbit;
		BinaryArray(Size_t _size, const T* _array, const Size_t _pack, const Size_t _offset, const Size_t _maxbit)
			: array(_array), size(_size), pack(_pack),  offset(_offset), maxbit(_maxbit) {}
		friend std::ostream& operator<<(std::ostream &stream, const BinaryArray &v) {
				Size_t        ibit = 0, rbit = 0; // rbit = numéro relatif du bit
				const Size_t  nbit = 8*sizeof(T);
				for(Size_t k=0;k<v.size;++k) { // sur tous les éléments du tableau
					T   mask = 1;
					for(Size_t i=0;i<nbit;++i,++ibit,mask<<=1) {
						if (ibit >= v.offset) {
								stream << (v.array[k] & mask ? '1' : '0');
								rbit = ibit - v.offset + 1;
								if (rbit == v.maxbit) break;
								if (rbit % v.pack == 0) stream << ' ';
						}
					}
					if (rbit == v.maxbit) break;
				}
				return stream;
		}
	};

	/// class Bits::Stream
	/// classe de gestions d'entrée/sortie de bits
	class Stream {
    public:
        /// type sous-jacent de stockage pour le stream
        using  storage_type = uint32_t;
        /// constantes de classe
        enum SizeConstants : Size_t {
            /// nombre de bits qui peuvent être stockés dans le type sous-jacent
            storage_unit_size = 8 * sizeof(storage_type),
            /// pas de réallocation de la zone de données si elle a besoin d'être agrandie
            alloc_unit_size = 256
        };
    public:
        // position d'un curseur
        class Position {
        protected:
            Size_t
                iBlock = 0,  ///< indice du bloc
                iBit = 0;    ///< indice du bit dans le bloc
        public:
            /// constructeur par défault (= 0,0)
            inline Position() = default;
            /// construction à partir d'un numéro de bits
            inline Position(Size_t nBits) : iBlock(nBits / storage_unit_size), iBit(nBits % storage_unit_size) {}
            /// getters
            inline Size_t getBlock() const { return iBlock; }
            inline Size_t getBit() const { return iBit; }
			/// réinitialise le curseur
            inline void reset() { iBlock = iBit = 0; }
			/// avance le curseur de 1 bit
            inline Size_t next() {
				++iBit;
				if (iBit == storage_unit_size) { iBit = 0; ++iBlock; }
				return iBlock;
			}
			/// taille
            inline Size_t LastBlock() const { return iBlock + (iBit ? 1 : 0); }
            inline Size_t LastByte() const { return Size_t(sizeof(storage_type))*iBlock + (iBit+7)/8; }
            inline Size_t LastBit() const { return iBlock*storage_unit_size + iBit; }
			/// se place à nbits depuis le début du stream
            /// 0 = premier bit
            inline void seek(const Size_t nBits) {
				iBlock = nBits / storage_unit_size;
				iBit   = nBits % storage_unit_size;
            }
			// affichage
			friend std::ostream& operator<<(std::ostream& os, const Position &pos) {
				return os << pos.iBlock << "/" << pos.iBit;
			}
			// opérateur égal
			friend bool operator==(const Position& Pos1, const Position& Pos2) {
				return (Pos1.iBlock == Pos2.iBlock) && (Pos1.iBit == Pos2.iBit);
			}
			friend bool operator!=(const Position& Pos1, const Position& Pos2) {
				return (Pos1.iBlock != Pos2.iBlock) || (Pos1.iBit != Pos2.iBit);
			}
			friend class Stream;
            friend Stream& operator<<(Stream &stream, const Bit &bit);
            friend bool operator>>(Stream &stream, Bit &b);
            friend bool operator==(const Stream& stream1, const Stream& stream2);
        };
	protected:
		/// taille de la zone de données réservée
		Size_t			storage_size;
        /// pointeurs d'écriture et de lecture
        Position        WritePosition,ReadPosition;
        /// pointeur vers la zone de données
        storage_type	*buff;
        /// méthode interne de réallocation
		inline void realloc(Size_t new_size) {
            storage_type	*tmp = new storage_type[new_size];
            if (buff != nullptr) memcpy(tmp, buff, storage_size*sizeof(storage_type));
            delete[] buff;
            buff = tmp;
            storage_size = new_size;
		}
	public:
		///@name gestion de la place mémoire pour le stream
		///@{
		/// retourne la place mémoire réservée pour le stream (en unité du type sous-jacent de stockage)
		inline Size_t	get_storage_size() const { return storage_size; }
		/// retourne la place mémoire réservée pour le stream en octets.
		inline Size_t	get_storage_byte_size() const { return Size_t(sizeof(storage_type)) * get_storage_size(); }
		/// retourne la place mémoire réservée pour le stream en bits.
		inline Size_t	get_storage_bit_size() const { return 8 * get_storage_byte_size(); }

		/// realloue la place mémoire allouée pour le stream s'il n'est pas assez grand pour stocker Request bytes.
		/// Donc, le laisse inchangé si la place mémoire allouée est déjé assez grande.
        inline void request_storage_size(Size_t request_size_in_byte) {
			Size_t  allocated_size = storage_size * Size_t(sizeof(storage_type));
			if (request_size_in_byte < allocated_size) return;
			Size_t  allocation_block_size = alloc_unit_size * Size_t(sizeof(storage_type));
			Size_t  nb_blocks = (request_size_in_byte + allocation_block_size - 1 ) / allocation_block_size;
			realloc( nb_blocks * allocation_block_size );
		}
		///@}

		/// constructeur. L'argument est la taille par défait de la zone de stockage
		inline Stream(const Size_t BitSize = alloc_unit_size * storage_unit_size) :
            storage_size(BitSize/storage_unit_size + (BitSize%storage_unit_size?1:0)),
            WritePosition(), ReadPosition(),
            buff( new storage_type[storage_size] ) {}

		/// constructeur par copie
		inline Stream(const Stream& s):
            storage_size(s.storage_size),
            WritePosition(s.WritePosition), ReadPosition(s.ReadPosition),
			buff( s.storage_size ? new storage_type[s.storage_size] : nullptr ) {
			Size_t  memsize = WritePosition.LastByte();
			if (memsize) memcpy((void*)buff,(void*)s.buff,memsize);
		}
		/// constructeur par déplacement
		inline Stream(Stream&& s):
			storage_size(s.storage_size),
            WritePosition(s.WritePosition), ReadPosition(s.ReadPosition),
            buff(s.buff)
		{
			s.buff = nullptr;
			s.storage_size = 0;
			s.WritePosition.reset();
			s.ReadPosition.reset();
		}
		/// assignation par copie
		inline Stream& operator=(const Stream& origin) {
			if (this != &origin) {
				Size_t   origin_size = origin.WritePosition.LastBlock();
				if (storage_size < origin_size) request_storage_size(origin_size);
				if (origin_size) memcpy((void*)buff,(void*)origin.buff,origin_size*sizeof(storage_type));
				WritePosition = origin.WritePosition;
				ReadPosition = origin.ReadPosition;
			}
			return *this;
		}
		/// assignation par déplacement
		inline Stream& operator=(Stream&& origin) {
			if (this != &origin) {
				std::swap(buff,origin.buff);
				std::swap(storage_size,origin.storage_size);
				std::swap(WritePosition,origin.WritePosition);
				std::swap(ReadPosition,origin.ReadPosition);
			}
			return *this;
		}
        /// destructeur
        inline ~Stream() {
            delete[] buff;
            buff = nullptr;
        }

		/// affiche les positions des pointeurs de lecture et d'écriture
		inline void status() const {
			std::cout << "status:"
				<< " Write= " << WritePosition
				<< " Read= "  << ReadPosition
				<< std::endl;
		}

		/// efface les données de la zone de stockage (= les mets à zéro) et
		/// réinitialise au début de la zone les pointeurs de lecteur et d'écriture.
		/// En résultat, le flux est vide. La mémoire déjà allouée n'est pas modifiée.
		/// Utile pour recycler un flux.
		inline void clear() {
			reset();
			memset(buff, 0, storage_size*sizeof(storage_type));
		}

		///@name information sur le flux (pour lecture/écriture de fichiers)
		///@{
		/// remise à zéro des pointeurs de lecture et d'écriture
		inline void reset() {
			WritePosition.reset();
			ReadPosition.reset();
		}
		/// retour du pointeur sur le buffer de données
		inline storage_type	*get_data() const { return buff; }
		/// retourne un pointeur char* vers les données du stream.
		inline char *get_buffer() const { return (char*)(buff); }
		/// retourne le nombre d'unité de stockage occupé par les données dans le stream
        /// entre le début du stream et la position du pointeur d'écriture.
		inline Size_t get_size() const { return WritePosition.LastBlock(); }
		/// retourne le nombre d'octets occupés par les données dans le stream
        /// entre le début du stream et la position du pointeur d'écriture.
		inline Size_t get_byte_size() const { return WritePosition.LastByte(); }
		/// retourne le nombre de bits occupés par les données dans le stream
        /// entre le début du stream et la position du pointeur d'écriture.
		inline Size_t get_bit_size() const { return WritePosition.LastBit(); }
		/// fixe la position du curseur de lecture. Utile après avoir rechargé les données dans
		/// le flux depuis une source externe. Le paramètre nBits est le nombre de bits écrit dans
		/// le flux.
        /// réinitialise la position de lecture.
		inline bool write_seek(const Size_t ibit) {
			if ( ibit >= get_storage_bit_size() ) return false;
			WritePosition.seek(ibit);
            ReadPosition.reset();
            return true;
		}
		///@}

		///@name méthodes utiles pour la lecture des données dans le flux.
		///@{
		/// @brief déplacement du pointeur de lecture en bit depuis le début du flux
		/// @detail L'appel seek() ramène le pointeur de lecture au début du flux.
		/// offset est l'offset en bit depuis le début du flux.
		/// Retourne vrai si l'opération a réussi.
		inline bool seek(Size_t ibit=0) {
			Size_t    maxbits = WritePosition.LastBit();
			if (ibit >= maxbits) return false;
			ReadPosition.seek(ibit);
            return true;
		}
 		///@brief déplacement du pointeur de lecture en bit depuis le fin du flux
		///@detail L'appel seek_end() amène le pointeur de lecture à la fin du flux.
        /// Retourne vrai si l'opération a réussi.
		inline bool seek_end(Size_t ebit=0) {
            Size_t    maxbits = WritePosition.LastBit() - 1;
            if (ebit > maxbits) return false;
			Size_t ibit = maxbits - ebit;
			return seek(ibit);
		}

	  /// vrai si le pointeur de lecture a atteint la fin des données écrites
		inline bool	end_of_stream() const { return ReadPosition == WritePosition; }
		/// récupère la position du curseur d'écriture
		inline const Position& getWritePosition() const { return WritePosition; }
        /// récupère la position du curseur de lecture
        inline const Position& getReadPosition() const { return ReadPosition; }

	  ///@}

		///@name surcharge des opérateurs pour lecture/écriture dans le stream
		/// attention: les opérateurs >> renvoient toujours le nombre de bits lus.
		///@{
		/// surcharge opérateur de stream pour les bits.
		/// écriture d'un bit
		friend Stream& operator<<(Stream &stream, const Bit &bit) {
			Stream::Position&  pos = stream.WritePosition;
			stream.buff[pos.iBlock]
					= Bits::set<storage_type>(stream.buff[pos.iBlock], pos.iBit, 1, bit);
			if (pos.next() == stream.storage_size)
				stream.realloc(stream.storage_size + stream.alloc_unit_size);
			return stream;
		}
		/// surcharge opérateur de stream pour les bits.
		/// lecture d'un bit
		friend bool operator>>(Stream &stream, Bit &b) {
			if (stream.ReadPosition == stream.WritePosition) return false;
			Stream::Position&  pos = stream.ReadPosition;
			b = Bits::get<storage_type>(stream.buff[pos.iBlock], pos.iBit);
			pos.next();
			return true;
		}

		/// surcharge opérateur de stream pour les Bits:Block.
		/// écriture d'un BitsBlock
		template <int NBITS> friend
			Stream&	operator<<(Stream &stream, const Block<NBITS> &bitblock) {
				Size_t	 valid = bitblock.get_valid() - 1;
				for(Size_t i = 0;i <= valid;++i) {
					Bit  bit = get<typename Block<NBITS>::Type>(bitblock.get(), valid-i);
					stream << bit;
				}
				return stream;
		};
		/// surcharge opérateur de stream pour les Bits:Block.
		/// lecture d'un BitsBlock
		template <int NBITS> friend
			Size_t operator>>(Stream &stream, Block<NBITS> &bitblock) {
				Size_t	count = 0, valid = bitblock.get_valid() - 1;
				for(Size_t i = 0; i <= valid; ++i) {
					Bit  bit;
					bool read_bit = stream >> bit;
					if (read_bit)	{
						bitblock.set( Bits::set<typename Block<NBITS>::Type>(bitblock.get(), valid-i, 1, bit) );
						++count;
					} else  {
						// complète le reste de la lecture avec des 0
						for(Size_t j=i;j<=valid;++j)
							bitblock.set( Bits::set<typename Block<NBITS>::Type>(bitblock.get(), valid-j, 1, 0));
						break; // plus de bits à lire (fin de flux)
					}
				}
				return count;
		}

		/// surcharge opérateur de stream pour les Bits:Block.
		/// écriture d'un BitsBlock
		friend Stream&	operator<<(Stream &stream, const varBlock &bitblock) {
			Size_t	 valid = bitblock.get_valid() - 1;
			for(Size_t i=0;i<=valid;++i) {
				Bit  bit = get<typename varBlock::Type>(bitblock.get(), valid-i);
				stream << bit;
			}
			return stream;
		};
		/// surcharge opérateur de stream pour les Bits:Block.
		/// lecture d'un BitsBlock
		friend	Size_t operator>>(Stream &stream, varBlock &bitblock) {
				Size_t	count = 0, valid = bitblock.get_valid() - 1;
				for(Size_t  i = 0; i <= valid; ++i) {
					Bit  bit;
					bool read_bit = stream >> bit;
					if (read_bit)	{
						bitblock.set( Bits::set<varBlock::Type>(bitblock.get(), valid-i, 1, bit) );
						++count;
					} else  {
						// complète le reste de la lecture avec des 0
						for(Size_t j=i;j<=valid;++j)
							bitblock.set( Bits::set<varBlock::Type>(bitblock.get(), valid-j, 1, 0));
						break; // plus de bits à lire (fin de flux)
					}
				}
				return count;
		}

        /// surcharge de l'opérateur == pour comparer deux flux;
        /// même position des curseurs en écriture et même données
        friend bool operator==(const Stream& stream1, const Stream& stream2) {
            bool bWPos = (stream1.WritePosition == stream2.WritePosition);
            if (!bWPos) return false;
            bool            bBlockData = true;
            const Position  &wPos = stream1.WritePosition;
			if (wPos.iBlock) {
                int diff = memcmp((void*)stream1.buff,(void*)stream2.buff,wPos.iBlock*sizeof(storage_type));
                bBlockData = (diff == 0);
            }
            storage_type  end_mask = mask<storage_type>(0,wPos.iBit);
            bool bTrailData = ((stream1.buff[wPos.iBlock] & end_mask) ==
                          (stream2.buff[wPos.iBlock] & end_mask));
            return bBlockData && bTrailData;
        }


        /// surcharge de l'opérateur != pour comparer deux flux;
        /// retourne faux si les flux diffèrent par leurs positions d'écriture ou de lecture,
        /// ou par leurs données.
        friend bool operator!=(const Stream& stream1, const Stream& stream2) {
            bool bRPos = (stream1.ReadPosition == stream2.ReadPosition);
            return !(bRPos && (stream1 == stream2));
        }


		/// surcharge opérateur de stream pour les BITs (uintXX_t)
		/// écriture d'un uintXX_t
		template <typename T> friend
			Stream&	operator<<(Stream &stream, const T &data) {
				using uType = typename uTypeImpl<sizeof(T)>::Type;
				const uType	&udata = *reinterpret_cast<const uType*>(&data);
				Bits::Block<Size_t(8*sizeof(T))>	bitblock(udata);
				stream << bitblock;
				return stream;
		}
		/// surcharge opérateur de stream pour les BITs (uintXX_t)
		/// lecture d'un uintXX_t
		template <typename T> friend
			Size_t operator>>(Stream &stream, T &data) {
				using uType = typename uTypeImpl<sizeof(T)>::Type;
				uType	&udata = *reinterpret_cast<uType*>(&data);
				Bits::Block<Size_t(8*sizeof(T))>	bitblock;
				Size_t count = stream >> bitblock;
				udata = bitblock.get();
				return count;
		};
		///@}

		/// @brief fonction à utiliser pour affichage en binaire d'un objet dans le flux.
		/// @param pack groupe les bits par paquets de pack (0 = pas de packing)
		/// @param offset commence à offset bit depuis le début de l'objet. Les bits avant l'offset ne sont pas affiché (0 = pas d'offset).
		/// @param maxbit affiche maxbit au maximum (0 = tous).
		friend BinaryArray<Stream::storage_type>
			Binary(const Stream& stream, const Size_t pack=0, const Size_t offset=0, const Size_t maxbit=0) {
				return BinaryArray<Stream::storage_type>(
					stream.WritePosition.LastBlock(),
					stream.buff,
					pack,offset,(maxbit?maxbit:stream.get_bit_size()-offset));
			}

		///=================================================================================================
		/// surcharge pour affichage direct d'un stream dans un flux de sortie
		/// @param os		le flux de sortie
		/// @param stream	 	le flux binaire à afficher
		friend std::ostream& operator<<(std::ostream& os, const Stream& stream) {
			return os << BinaryArray<Stream::storage_type>(
				stream.WritePosition.LastBlock(),
				stream.buff,
				stream.get_bit_size(), 0, stream.get_bit_size());
		}

	};

}

#undef WARNING
#undef DEBUG
#endif
