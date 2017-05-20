/*
 * Copyright (C) Telecom ParisTech
 * 
 * This file must be used under the terms of the CeCILL. This source
 * file is licensed as described in the file COPYING, which you should
 * have received as part of this distribution. The terms are also
 * available at:
 * http://www.cecill.info/licences/Licence_CeCILL_V1.1-US.txt
*/

#include <python2.7/Python.h>

#include "des.h"
#include "utils.h"

extern PyObject *InternalError;

static inline PyObject * standard_wrapper (uint64_t (*f) (uint64_t), PyObject *self, PyObject *args) {
    uint64_t arg, result;

    if (!PyArg_ParseTuple(args, "k", &arg))
        return NULL;

    result = (*f) (arg);
    if (PyErr_Occurred ())
        return NULL;

    return Py_BuildValue("k", result);
}
    
#define CREATE_WRAPPER(f)  \
static PyObject * f ## _wrapper (PyObject *self, PyObject *args) {  \
    return standard_wrapper (f, self, args);    \
}

CREATE_WRAPPER ( des_ip )
CREATE_WRAPPER ( des_n_ip )
CREATE_WRAPPER ( des_fp )
CREATE_WRAPPER ( des_n_fp )
CREATE_WRAPPER ( des_e )
CREATE_WRAPPER ( des_n_e )
CREATE_WRAPPER ( des_p )
CREATE_WRAPPER ( des_n_p )
CREATE_WRAPPER ( des_pc1 )
CREATE_WRAPPER ( des_set_parity_bits )
CREATE_WRAPPER ( des_n_pc1 )
CREATE_WRAPPER ( des_pc2 )
CREATE_WRAPPER ( des_n_pc2 )
CREATE_WRAPPER ( des_sboxes )
CREATE_WRAPPER ( des_right_half )
CREATE_WRAPPER ( des_left_half )
CREATE_WRAPPER ( des_ls )
CREATE_WRAPPER ( des_rs )

static PyObject * des_sbox_wrapper (PyObject *self, PyObject *args) {
    uint64_t val, result;
    int sbox;
    if (!PyArg_ParseTuple(args, "ik", &sbox, &val))
        return NULL;

    result = des_sbox (sbox, val);
    if (PyErr_Occurred ())
        return NULL;

    return Py_BuildValue("k", result);
}

static PyObject * des_f_wrapper (PyObject *self, PyObject *args) {
    uint64_t rk, val, result;
    if (!PyArg_ParseTuple(args, "kk", &rk, &val))
        return NULL;

    result = des_f (rk, val);
    if (PyErr_Occurred ())
        return NULL;

    return Py_BuildValue("k", result);
}

static PyObject * des_ks_wrapper (PyObject *self, PyObject *args) {
    uint64_t val;
    uint64_t result[16];
    if (!PyArg_ParseTuple(args, "k", &val))
        return NULL;

    des_ks (result, val);
    if (PyErr_Occurred ())
        return NULL;

    PyObject * ks = PyList_New(16);
    if (ks == NULL)
        return NULL;

    int i;
    for (i=0; i<16; i++) {
        PyObject * temp = Py_BuildValue("k", result[i]);
        if (PyList_SetItem(ks, i, temp) != 0)
            return NULL;
    }

    return ks;
}

static PyObject * des_enc_wrapper (PyObject *self, PyObject *args) {
    uint64_t ks[16], val, result;
    PyObject *obj, *item;
    if (!PyArg_ParseTuple(args, "Ok", &obj, &val))
        return NULL;
    if (!PyList_Check (obj)) {
        PyErr_SetString(PyExc_TypeError, "key schedule should be a list of 16 numbers");
        return NULL;
    }
    if (PyList_Size (obj) != 16) {
        PyErr_SetString(PyExc_TypeError, "key schedule should be a list of 16 numbers");
        return NULL;
    }

    Py_ssize_t i;
    for (i=0; i<16; i++) {
        item = PyList_GetItem (obj, i);
        if (!item)
            return NULL;
        if (!PyNumber_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "key schedule should be a list of 16 numbers");
            return NULL;
        }

        item = PyNumber_Long(item);
        ks [i] = PyLong_AsUnsignedLong(item);
        Py_DECREF (item);

        if (PyErr_Occurred())
            return NULL;
    }

    result = des_enc (ks, val);
    if (PyErr_Occurred ())
        return NULL;

    return Py_BuildValue("k", result);
}

static PyObject * des_dec_wrapper (PyObject *self, PyObject *args) {
    uint64_t ks[16], val, result;
    PyObject *obj, *item;
    if (!PyArg_ParseTuple(args, "Ok", &obj, &val))
        return NULL;
    if (!PyList_Check (obj)) {
        PyErr_SetString(PyExc_TypeError, "key schedule should be a list of 16 numbers");
        return NULL;
    }
    if (PyList_Size (obj) != 16) {
        PyErr_SetString(PyExc_TypeError, "key schedule should be a list of 16 numbers");
        return NULL;
    }

    Py_ssize_t i;
    for (i=0; i<16; i++) {
        item = PyList_GetItem (obj, i);
        if (!item)
            return NULL;
        if (!PyNumber_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "key schedule should be a list of 16 numbers");
            return NULL;
        }

        item = PyNumber_Long(item);
        ks [i] = PyLong_AsUnsignedLong(item);
        Py_DECREF (item);

        if (PyErr_Occurred())
            return NULL;
    }

    result = des_dec (ks, val);
    if (PyErr_Occurred ())
        return NULL;

    return Py_BuildValue("k", result);
}

static PyObject * des_check_wrapper (PyObject *self, PyObject *args) {
    int result;

    result = des_check ();
    if (PyErr_Occurred ())
        return NULL;

    if (result)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

static PyMethodDef DESMethods[] = {
    {"ip",  des_ip_wrapper, METH_VARARGS,
     "Initial permutation (64 to 64 bits). Same as n_fp().\n"
     "\n"
     "Args:\n"
     "    val (long): 64 bits input\n"
     "\n"
     "Returns:\n"
     "    The permutated input as a 64 bits integer\n\n"},
    {"n_ip",  des_n_ip_wrapper, METH_VARARGS,
     "Inverse of initial permutation (64 to 64 bits). Same as fp().\n"
     "\n"
     "Args:\n"
     "    val (long): 64 bits input\n"
     "\n"
     "Returns:\n"
     "    The permutated input as a 64 bits integer\n\n"},
    {"fp",  des_fp_wrapper, METH_VARARGS,
     "Final permutation (64 to 64 bits). Inverse of initial permutation, same\n"
     "as n_ip().\n"
     "\n"
     "Args:\n"
     "    val (long): 64 bits input\n"
     "\n"
     "Returns:\n"
     "    The permutated input as a 64 bits integer\n\n"},
    {"n_fp",  des_n_fp_wrapper, METH_VARARGS,
     "Inverse of final permutation (64 to 64 bits). Same as ip().\n"
     "\n"
     "Args:\n"
     "    val (long): 64 bits input\n"
     "\n"
     "Returns:\n"
     "    The permutated input as a 64 bits integer\n\n"},
    {"e",  des_e_wrapper, METH_VARARGS,
     "E expansion - permutation (32 to 48 bits).\n"
     "\n"
     "Args:\n"
     "    val (long): 32 bits input\n"
     "\n"
     "Returns:\n"
     "    The expanded and permutated input as a 48 bits integer\n\n"},
    {"n_e",  des_n_e_wrapper, METH_VARARGS,
     "Inverse of E expansion - permutation (48 to 32 bits). Duplicated bits\n"
     "must have the same value. If they do not, an error is raised.\n"
     "\n"
     "Args:\n"
     "    val (long): 48 bits input\n"
     "\n"
     "Returns:\n"
     "    The permutated and selected input as a 32 bits integer\n\n"},
    {"p",  des_p_wrapper, METH_VARARGS,
     "P permutation (32 to 32 bits).\n"
     "\n"
     "Args:\n"
     "    val (long): 32 bits input\n"
     "\n"
     "Returns:\n"
     "    The permutated input as a 32 bits integer\n\n"},
    {"n_p",  des_n_p_wrapper, METH_VARARGS,
     "Inverse of P permutation (32 to 32 bits).\n"
     "\n"
     "Args:\n"
     "    val (long): 32 bits input\n"
     "\n"
     "Returns:\n"
     "    The permutated input as a 32 bits integer\n\n"},
    {"pc1",  des_pc1_wrapper, METH_VARARGS,
     "PC1 permutation - selection (64 to 56 bits). No parity check.\n"
     "\n"
     "Args:\n"
     "    val (long): 64 bits input\n"
     "\n"
     "Returns:\n"
     "    The permutated and selected input as a 56 bits integer\n\n"},
    {"set_parity_bits",  des_set_parity_bits_wrapper, METH_VARARGS,
     "Computes the 8 parity bits of a 64 bits word. Parity bits are the\n"
     "rightmost bit of each byte. Once computed, the number of set bits of\n"
     "each byte is odd, as specified in the DES standard.\n"
     "\n"
     "Args:\n"
     "    val (long): 64 bits input\n"
     "\n"
     "Returns:\n"
     "    The input with odd parity bits, as a 64 bits integer\n\n"},
    {"n_pc1",  des_n_pc1_wrapper, METH_VARARGS,
     "Inverse of PC1 permutation - selection (56 to 64 bits). Parity bits\n"
     "are computed.\n"
     "\n"
     "Args:\n"
     "    val (long): 56 bits input\n"
     "\n"
     "Returns:\n"
     "    The permutated and expanded input as a 64 bits integer\n\n"},
    {"pc2",  des_pc2_wrapper, METH_VARARGS,
     "PC2 permutation - selection (56 to 48 bits).\n"
     "\n"
     "Args:\n"
     "    val (long): 56 bits input\n"
     "\n"
     "Returns:\n"
     "    The permutated and selected input as a 48 bits integer\n\n"},
    {"n_pc2",  des_n_pc2_wrapper, METH_VARARGS,
     "Inverse of PC2 permutation - selection (48 to 56 bits). Missing\n"
     "bits are set to 0.\n"
     "\n"
     "Args:\n"
     "    val (long): 48 bits input\n"
     "\n"
     "Returns:\n"
     "    The permutated and expanded input as a 56 bits integer\n\n"},
    {"sbox",  des_sbox_wrapper, METH_VARARGS,
     "Single SBox computation (6 to 4 bits).\n"
     "\n"
     "Args:\n"
     "    sbox (int): SBox number, from 1 to 8\n"
     "    val (long): 6 bits input\n"
     "\n"
     "Returns:\n"
     "    The 4 bits output of SBox number `sbox` corresponding to the\n"
     "    6 bits input, as a 4 bits integer\n\n"},
    {"sboxes",  des_sboxes_wrapper, METH_VARARGS,
     "All SBoxes computation (48 to 32 bits).\n"
     "\n"
     "Args:\n"
     "    val (long): 48 bits input\n"
     "\n"
     "Returns:\n"
     "    The 32 bits output of all SBoxes corresponding to the 48 bits\n"
     "    input, as a 32 bits integer\n\n"},
    {"right_half",  des_right_half_wrapper, METH_VARARGS,
     "Returns the 32 bits right half of a 64 bits word.\n"
     "\n"
     "Args:\n"
     "    val (long): 64 bits input\n"
     "\n"
     "Returns:\n"
     "    The 32 bits right half of a 64 bits word, as a 32 bits integer\n\n"},
    {"left_half",  des_left_half_wrapper, METH_VARARGS,
     "Returns the 32 bits left half of a 64 bits word.\n"
     "\n"
     "Args:\n"
     "    val (long): 64 bits input\n"
     "\n"
     "Returns:\n"
     "    The 32 bits left half of a 64 bits word, as a 32 bits integer\n\n"},
    {"ls",  des_ls_wrapper, METH_VARARGS,
     "Applies the **left shift** rotation of the standard (56 to 56\n"
     "bits).\n"
     "\n"
     "Args:\n"
     "    val (long): 56 bits input\n"
     "\n"
     "Returns:\n"
     "    The rotated input, as a 56 bits integer\n\n"},
    {"rs",  des_rs_wrapper, METH_VARARGS,
     "Applies the **right shift** rotation of the standard (56 to 56\n"
     "bits).\n"
     "\n"
     "Args:\n"
     "    val (long): 56 bits input\n"
     "\n"
     "Returns:\n"
     "    The rotated input, as a 56 bits integer\n\n"},
    {"f",  des_f_wrapper, METH_VARARGS,
     "The F function of the standard (48+32 bits to 32 bits).\n"
     "\n"
     "Args:\n"
     "    rk (long): 48 bits round key\n"
     "    val (long): 32 bits data input\n"
     "\n"
     "Returns:\n"
     "    The transformed input as a 32 bits integer\n\n"},
    {"ks",  des_ks_wrapper, METH_VARARGS,
     "Computes the whole key schedule from a 64 bits secret key and\n"
     "stores the sixteen 48 bits round keys in a list.\n"
     "\n"
     "Args:\n"
     "    val (long): 64 bits key\n"
     "\n"
     "Returns:\n"
     "    The sixteen 48 bits round keys in a list\n\n"},
    {"enc",  des_enc_wrapper, METH_VARARGS,
     "Enciphers a 64 bits plaintext with a pre-computed key schedule.\n"
     "\n"
     "Args:\n"
     "    ks (List[long]): The pre-computed key schedule\n"
     "    val (long): The 64 bits plaintext\n"
     "\n"
     "Returns:\n"
     "    The enciphered plaintext as a 64 bits integer\n\n"},
    {"dec",  des_dec_wrapper, METH_VARARGS,
     "Deciphers a 64 bits ciphertext with a pre-computed key schedule.\n"
     "\n"
     "Args:\n"
     "    ks (List[long]): The pre-computed key schedule\n"
     "    val (long): The 64 bits ciphertext\n"
     "\n"
     "Returns:\n"
     "    The deciphered ciphertext as a 64 bits integer\n\n"},
    {"check",  des_check_wrapper, METH_VARARGS,
     "A functional verification of the DES implementation. Runs a\n"
     "number of encipherments with enc() and the corresponding\n"
     "decipherments with dec() and checks the results against\n"
     "pre-computed plaintexts, ciphertexts and secret keys.\n"
     "\n"
     "Returns:\n"
     "    `True` on success, `False` on errors\n\n"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC initdes(void) {
    PyObject * m = Py_InitModule3("des", DESMethods,
                  "The DES library, a software library dedicated to the Data Encryption\n"
                  "Standard (DES).\n"
                  "\n"
                  ":Date: 2009-07-08\n"
                  ":Authors:\n"
                  "    - Renaud Pacalet, renaud.pacalet@telecom-paristech.fr\n"
                  "\n"
                  "Attention\n"
                  "=========\n"
                  "\n"
                  "1. Most functions of the des library check their input parameters and\n"
                  "issue warnings or errors when they carry invalid values. Warnings are\n"
                  "printed on the standard error output. Errors are also printed on the\n"
                  "standard error output and the program exits with a -1 exit status.\n"
                  "2. The des library uses a single data type to represent all the data of\n"
                  "the DES standard: `uint64_t`. It is a 64 bits unsigned integer.\n"
                  "3. Data are always right aligned: when the data width is less than 64 bits,\n"
                  "the meaningful bits are always the rightmost bits of the `uint64_t`.\n\n");
    if (m == NULL)
        return;

    PyObject * Pyleft_shifts = PyList_New(16);
    if (Pyleft_shifts == NULL)
        return;

    int i;
    for (i=0; i<16; i++) {
        PyObject * temp = Py_BuildValue("i", left_shifts[i]);
        if (PyList_SetItem(Pyleft_shifts, i, temp) != 0)
            return;
    }

    PyObject_SetAttrString (m, "left_shifts", Pyleft_shifts);

    Py_DECREF (Pyleft_shifts);

    InternalError = PyErr_NewException("des.DESError", NULL, NULL);
    Py_INCREF(InternalError);
    PyModule_AddObject(m, "DESError", InternalError);
}
