/*
 * Copyright (C) Telecom ParisTech
 * 
 * This file must be used under the terms of the CeCILL. This source
 * file is licensed as described in the file COPYING, which you should
 * have received as part of this distribution. The terms are also
 * available at:
 * http://www.cecill.info/licences/Licence_CeCILL_V1.1-US.txt
*/

/** \file km.h
 *  The \b km library, a data structure and a set of functions to manage the  partial knowledge about a DES (Data Encryption Standard) secret key.
 *  \author Renaud Pacalet, renaud.pacalet@telecom-paristech.fr
 *  \date 2009-07-08
 */

#ifndef KM_H
#define KM_H

#include <stdint.h>
#include <inttypes.h>

/** A data structure to manage the partial knowledge we have about a 64 bits secret key KEY. */
struct des_key_manager_s
{
  /** The 56 bits word representing C0D0 = C16D16 = PC1(KEY). */
  uint64_t key;
  /** The 56 bits word in which each set bit indicates a known bit. */
  uint64_t mask;
};

typedef struct des_key_manager_s des_key_manager;

/** Allocates and initializes a key manager with zero-knowledge.
 * \return An initialized key manager. */
des_key_manager *des_km_init (void);

/** Return the number of known bits.
 * \return The number of known bits. */
int des_km_known (des_key_manager *km /**< The key manager. */ );

/** Injects in a key manager a known value of a 6 bits subkey of a 48 bits round key.
 * \return Zero if a conflict was detected between the new bit values and some previously known ones, a non zero value if there was no conflict. */
int des_km_set_sk (des_key_manager *km /**< The key manager. */ ,
		   int rk
		   /**< The round key number (1 for the first to 16 for the last). */
		   , int sk
		   /**< The subkey number (1 for the leftmost to 8 for the righmost). */
		   ,
    /** A flag specifying what to do in case the injected knowledge conflicts with previous knowledge. If non-zero, the new bit values overwrite the old conflicting values. If zero, the old values are unchanged. */
		   int force,
    /** A 6 bits mask indicating which bits of the subkey are injected: only these which corresponding mask bit is set (equal to 1) are injected. */
		   uint64_t mask, uint64_t val
		 /**< The 6 bits subkey to inject. */
  );

/** Injects in a key manager a known value of a 48 bits round key.
 * \return Zero if a conflict was detected between the new bit values and some previously known ones, a non zero value if there was no conflict. */
int des_km_set_rk (des_key_manager *km /**< The key manager. */ ,
		   int rk
		   /**< The round key number (1 for the first to 16 for the last). */
		   ,
    /** A flag specifying what to do in case the injected knowledge conflicts with previous knowledge. If non-zero, the new bit values overwrite the old conflicting values. If zero, the old values are unchanged. */
		   int force,
    /** A 48 bits mask indicating which bits of the round key are injected: only these which corresponding mask bit is set (equal to 1) are injected. */
		   uint64_t mask, uint64_t val
		 /**< The 48 bits round key to inject. */
  );

/** Injects in a key manager a known value of a 56 bits secret key (after PC1 (Permutated Choice 1), that is, C0D0 or C16D16 of the DES standard).
 * \return Zero if a conflict was detected between the new bit values and some previously known ones, a non zero value if there was no conflict. */
uint64_t des_km_set_c0d0 (des_key_manager *km /**< The key manager. */ ,
    /** A flag specifying what to do in case the injected knowledge conflicts with previous knowledge. If non-zero, the new bit values overwrite the old conflicting values. If zero, the old values are unchanged. */
			  int force,
    /** A 56 bits mask indicating which bits of the round key are injected: only these which corresponding mask bit is set (equal to 1) are injected. */
			  uint64_t mask, uint64_t val
		 /**< The 56 bits secret key to inject. */
  );

/** Injects in a key manager a known value of a 64 bits secret key. No parity check.
 * \return Zero if a conflict was detected between the new bit values and some previously known ones, a non zero value if there was no conflict. */
int des_km_set_key (des_key_manager *km /**< The key manager. */ ,
    /** A flag specifying what to do in case the injected knowledge conflicts with previous knowledge. If non-zero, the new bit values overwrite the old conflicting values. If zero, the old values are unchanged. */
		    int force,
    /** A 64 bits mask indicating which bits of the round key are injected: only these which corresponding mask bit is set (equal to 1) are injected. */
		    uint64_t mask, uint64_t val
		 /**< The 64 bits secret key to inject. */
  );

/** Extracts from a key manager the knowledge about a subkey of a round key.
 * \return The subkey as a 6 bits uint64_t, and a mask indicating which bits are known as another 6 bits uint64_t, in the variable pointed to by the last parameter. Every set bit of the mask indicates a known bit. */
uint64_t des_km_get_sk (des_key_manager *km /**< The key manager. */ ,
			int rk
			/**< The round key number (1 for the first to 16 for the last). */
			, int sk
			/**< The subkey number (1 for the leftmost to 8 for the rightmost). */
			,
    /** The returned 6 bits mask indicating which bits are known: a bit is known if its corresponding mask bit is set (equal to 1). */
			uint64_t * mask);

/** Extracts from a key manager the knowledge about a round key.
 * \return The round key as a 48 bits uint64_t, and a mask indicating which bits are known as another 48 bits uint64_t, in the variable pointed to by the last parameter. Every set bit of the mask indicates a known bit. */
uint64_t des_km_get_rk (des_key_manager *km /**< The key manager. */ ,
			int rk
			/**< The round key number (1 for the first to 16 for the last). */
			,
    /** The returned 48 bits mask indicating which bits are known: a bit is known if its corresponding mask bit is set (equal to 1). */
			uint64_t * mask);

/** Extracts from a key manager the knowledge about a 56 bits secret key (after permutated choice 1 - PC1, that is, C0D0 or C16D16 of the DES standard).
 * \return The secret key as a 56 bits uint64_t, and a mask indicating which bits are known as another 56 bits uint64_t, in the variable pointed to by the last parameter. Every set bit of the mask indicates a known bit. */
uint64_t des_km_get_c0d0 (des_key_manager *km /**< The key manager. */ ,
    /** The returned 56 bits mask indicating which bits are known: a bit is known if its corresponding mask bit is set (equal to 1). */
			  uint64_t * mask);

/** Extracts from a key manager the knowledge about a 64 bits secret key.
 * \return The secret key with parity bits set as a 64 bits uint64_t, and a mask indicating which bits are known as another 64 bits uint64_t, in the variable pointed to by the last parameter. Every set bit of the mask indicates a known bit. */
uint64_t des_km_get_key (des_key_manager *km /**< The key manager. */ ,
    /** The returned 64 bits mask indicating which bits are known: a bit is known if its corresponding mask bit is set (equal to 1). */
			 uint64_t * mask);

/** Initializes an iterator over all the unknown bits of a key manager. Used in conjunction with the des_km_for_unknown() iterator, it offers a loop scheme over all possible values of a partially known secret key:
 * \code
 * des_key_manager *km;
 * uint64_t key, mask;
 * ...
 * des_km_init_for_unknown(km);
 * do {
 *   key = des_km_get_key(km, &mask);
 *   ...
 * } while(des_km_for_unknown(km));
 * \endcode
 * \attention
 * <ol>
 * <li>The number of iterations exponentially depends on the number of unknown bits. Use with care.</li>
 * <li>The iterator does not change the number of known bits. It only assigns values to the unknown bits. The mask indicating which bits are known and which are not is unmodified.</li>
 * <li>Setting new known bits while iterating is not recommended and could lead to unwanted behaviors.</li>
 * </ol> */
void des_km_init_for_unknown (des_key_manager *km /**< The key manager. */ );

/** Iterates over all the possible values of a partially known secret key of a key manager. Used in conjunction with the des_km_init_for_unknown(), it offers a loop scheme over all possible values of a partially known secret key:
 * \code
 * des_key_manager *km;
 * uint64_t key, mask;
 * ...
 * des_km_init_for_unknown(km);
 * do {
 *   key = des_km_get_key(km, &mask);
 *   ...
 * } while(des_km_for_unknown(km));
 * \endcode
 * \attention
 * <ol>
 * <li>The number of iterations exponentially depends on the number of unknown bits. Use with care.</li>
 * <li>The iterator does not change the number of known bits. It only assigns values to the unknown bits. The mask indicating which bits are known and which are not is unmodified.</li>
 * <li>Setting new known bits while iterating is not recommended and could lead to unwanted behaviors.</li>
 * </ol>
 * \return A zero value when the current value of the iterator is an already encountered one (indicating that the loop is over), else a non-zero value. */
uint64_t des_km_for_unknown (des_key_manager *km /**< The key manager. */ );

/** Deallocates a key manager. */
void des_km_free (des_key_manager *km);

#endif /** not KM_H */
