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
#include <python2.7/structmember.h>

#include "km.h"
#include "utils.h"

extern PyObject *InternalError;

#define KEY_MANAGER(km) ((des_key_manager *) (&km->key))

typedef struct {
    PyObject_HEAD
    uint64_t key;
    uint64_t mask;
    uint64_t stillRemaining;
} km_kmIteratorObject;

static PyObject * kmIterator_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
    uint64_t key_t, mask_t;

    if (!PyArg_ParseTuple(args, "kk", &key_t, &mask_t))
        return NULL;

    km_kmIteratorObject *self= (km_kmIteratorObject *) type->tp_alloc(type, 0);
    if (!self)
        return NULL;

    self->key = key_t;
    self->mask = mask_t;
    self->stillRemaining = 1;

    des_km_init_for_unknown (KEY_MANAGER (self));
    if (PyErr_Occurred())
        return NULL;

    return (PyObject *) self;
}

static void kmIterator_dealloc(km_kmIteratorObject *self) {
    Py_TYPE(self)->tp_free(self);
}

static PyObject * kmIterator_next(km_kmIteratorObject *self) {
    uint64_t key, mask;

    if (self->stillRemaining) {
        key = des_km_get_key (KEY_MANAGER(self), &mask);
        if (PyErr_Occurred())
            return NULL;

        self->stillRemaining = des_km_for_unknown (KEY_MANAGER (self));
        if (PyErr_Occurred())
            return NULL;

        return Py_BuildValue("k", key);
    }

    return NULL;
}

PyTypeObject km_kmIteratorType = {
    PyObject_HEAD_INIT(NULL)
    0,                              /*ob_size*/
    "km.kmIterator",                /*tp_name*/
    sizeof(km_kmIteratorObject),    /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)kmIterator_dealloc, /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_reserved */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,             /* tp_flags */
    "An iterator class to loop over all the unknown bits of a key manager.\n"
    "It offers a loop scheme over all possible values of a partially known\n"
    "secret key::\n"
    "\n"
    "keym = km.km ()\n"
    "...\n"
    "for key in keym.unknown ():\n"
    "    ...\n"
    "\n"
    "Attention\n"
    "=========\n"
    "\n"
    "The number of iterations exponentially depends on the number of unknown\n"
    "bits. Use with care\n\n",      /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    PyObject_SelfIter,              /* tp_iter */
    (iternextfunc)kmIterator_next,  /* tp_iternext */
    0,                              /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    0,                              /* tp_init */
    PyType_GenericAlloc,            /* tp_alloc */
    kmIterator_new,                 /* tp_new */
};

typedef struct {
    PyObject_HEAD
    uint64_t key;
    uint64_t mask;
} km_kmObject;

static void km_dealloc(km_kmObject* self) {
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject * km_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    km_kmObject *self;

    self = (km_kmObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->key = 0;
        self->mask = 0;
    }

    return (PyObject *)self;
}

static int km_init(km_kmObject *self, PyObject *args, PyObject *kwds) {
    des_key_manager *km = des_km_init ();
    memcpy (KEY_MANAGER (self), km, sizeof(*km));
    des_km_free (km);

    return 0;
}

static PyMemberDef km_members[] = {
    {"key", T_ULONG, offsetof(km_kmObject, key), 0,
     "the 56 bits word representing C0D0 = C16D16 = PC1(KEY)"},
    {"mask", T_ULONG, offsetof(km_kmObject, mask), 0,
     "the 56 bits word in which each set bit indicates a known bit"},
    {NULL}  /* Sentinel */
};

static PyObject * km_known_wrapper (km_kmObject *self) {
    int result;

    result = des_km_known (KEY_MANAGER (self));
    if (PyErr_Occurred ())
        return NULL;
    
    return Py_BuildValue ("i", result);
}

static PyObject * km_set_sk_wrapper (km_kmObject *self, PyObject *args) {
    int result, rk, sk, force;
    uint64_t mask, val;

    if (!PyArg_ParseTuple(args, "iiikk", &rk, &sk, &force, &mask, &val))
        return NULL;

    result = des_km_set_sk (KEY_MANAGER (self), rk, sk, force, mask, val);
    if (PyErr_Occurred ())
        return NULL;
    
    if (result)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

static PyObject * km_set_rk_wrapper (km_kmObject *self, PyObject *args) {
    int result, rk, force;
    uint64_t mask, val;

    if (!PyArg_ParseTuple(args, "iikk", &rk, &force, &mask, &val))
        return NULL;

    result = des_km_set_rk (KEY_MANAGER (self), rk, force, mask, val);
    if (PyErr_Occurred ())
        return NULL;
    
    if (result)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

static PyObject * km_set_c0d0_wrapper (km_kmObject *self, PyObject *args) {
    int force;
    uint64_t result, mask, val;

    if (!PyArg_ParseTuple(args, "ikk", &force, &mask, &val))
        return NULL;

    result = des_km_set_c0d0 (KEY_MANAGER (self), force, mask, val);
    if (PyErr_Occurred ())
        return NULL;
    
    if (result)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

static PyObject * km_set_key_wrapper (km_kmObject *self, PyObject *args) {
    int result, force;
    uint64_t mask, val;

    if (!PyArg_ParseTuple(args, "ikk", &force, &mask, &val))
        return NULL;

    result = des_km_set_key (KEY_MANAGER (self), force, mask, val);
    if (PyErr_Occurred ())
        return NULL;
    
    if (result)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

static PyObject * km_get_sk_wrapper (km_kmObject *self, PyObject *args) {
    int rk, sk;
    uint64_t mask, result;

    if (!PyArg_ParseTuple(args, "ii", &rk, &sk))
        return NULL;

    result = des_km_get_sk (KEY_MANAGER (self), rk, sk, &mask);
    if (PyErr_Occurred ())
        return NULL;
    
    return Py_BuildValue ("{s:k,s:k}", "key", result, "mask", mask);
}

static PyObject * km_get_rk_wrapper (km_kmObject *self, PyObject *args) {
    int rk;
    uint64_t mask, result;

    if (!PyArg_ParseTuple(args, "i", &rk))
        return NULL;

    result = des_km_get_rk (KEY_MANAGER (self), rk, &mask);
    if (PyErr_Occurred ())
        return NULL;
    
    return Py_BuildValue ("{s:k,s:k}", "key", result, "mask", mask);
}

static PyObject * km_get_c0d0_wrapper (km_kmObject *self) {
    uint64_t mask, result;

    result = des_km_get_c0d0 (KEY_MANAGER (self), &mask);
    if (PyErr_Occurred ())
        return NULL;
    
    return Py_BuildValue ("{s:k,s:k}", "key", result, "mask", mask);
}

static PyObject * km_get_key_wrapper (km_kmObject *self) {
    uint64_t mask, result;

    result = des_km_get_key (KEY_MANAGER (self), &mask);
    if (PyErr_Occurred ())
        return NULL;
    
    return Py_BuildValue ("{s:k,s:k}", "key", result, "mask", mask);
}

static PyObject * km_unknown (km_kmObject *self) {
    PyObject *argList = Py_BuildValue("kk", self->key, self->mask);

    PyObject *result = PyObject_CallObject((PyObject *) &km_kmIteratorType, argList);

    Py_DECREF(argList);

    return result;
}


static PyMethodDef km_methods[] = {
    {"known", (PyCFunction)km_known_wrapper, METH_NOARGS,
     "Return the number of known bits.\n"
     "\n"
     "Returns:\n"
     "    The number of known bits\n\n"},
    {"set_sk",  (PyCFunction)km_set_sk_wrapper, METH_VARARGS,
     "Injects in a key manager a known value of a 6 bits subkey of a 48 bits round\n"
     "key.\n"
     "\n"
     "Args:\n"
     "    rk (int): The round key number (1 for the first to 16 for the last)\n"
     "    sk (int): The subkey number (1 for the leftmost to 8 for the rightmost)\n"
     "    force (bool): A flag specifying what to do in case the injected knowledge\n"
     "                  conflicts with previous knowledge. If `True`, the new bit\n"
     "                  values overwrite the old conflicting values. If `False`, the\n"
     "                  old values are unchanged\n"
     "\n"
     "    mask (long): A 6 bits mask indicating which bits of the subkey are injected:\n"
     "                 only these which corresponding mask bit is set (equal to 1) are\n"
     "                 injected\n"
     "    val (long): The 6 bits subkey to inject\n"
     "\n"
     "Returns:\n"
     "    `False` if a conflict was detected between the new bit values and some\n"
     "    previously known ones, `True` if there was no conflict\n\n"},
    {"set_rk",  (PyCFunction)km_set_rk_wrapper, METH_VARARGS,
     "Injects in a key manager a known value of a 48 bits round key.\n"
     "\n"
     "Args:\n"
     "    rk (int): The round key number (1 for the first to 16 for the last)\n"
     "    force (bool): A flag specifying what to do in case the injected knowledge\n"
     "                  conflicts with previous knowledge. If `True`, the new bit\n"
     "                  values overwrite the old conflicting values. If `False`, the\n"
     "                  old values are unchanged\n"
     "    mask (long): A 48 bits mask indicating which bits of the round key are injected:\n"
     "                 only these which corresponding mask bit is set (equal to 1) are\n"
     "                 injected\n"
     "    val (long): The 48 bits round key to inject\n"
     "\n"
     "Returns:\n"
     "    `False` if a conflict was detected between the new bit values and some\n"
     "    previously known ones, `True` if there was no conflict\n\n"},
    {"set_c0d0",  (PyCFunction)km_set_c0d0_wrapper, METH_VARARGS,
     "Injects in a key manager a known value of a 56 bits secret key (after PC1\n"
     "(Permutated Choice 1), that is, C0D0 or C16D16 of the DES standard).\n"
     "\n"
     "Args:\n"
     "    force (bool): A flag specifying what to do in case the injected knowledge\n"
     "                  conflicts with previous knowledge. If `True`, the new bit\n"
     "                  values overwrite the old conflicting values. If `False`, the\n"
     "                  old values are unchanged\n"
     "    mask (long): A 56 bits mask indicating which bits of the secret key are injected:\n"
     "                 only these which corresponding mask bit is set (equal to 1) are\n"
     "                 injected\n"
     "    val (long): The 56 bits secret key to inject\n"
     "\n"
     "Returns:\n"
     "    `False` if a conflict was detected between the new bit values and some\n"
     "    previously known ones, `True` if there was no conflict\n\n"},
    {"set_key",  (PyCFunction)km_set_key_wrapper, METH_VARARGS,
     "Injects in a key manager a known value of a 64 bits secret key. No parity check.\n"
     "\n"
     "Args:\n"
     "    force (bool): A flag specifying what to do in case the injected knowledge\n"
     "                  conflicts with previous knowledge. If `True`, the new bit\n"
     "                  values overwrite the old conflicting values. If `False`, the\n"
     "                  old values are unchanged\n"
     "    mask (long): A 64 bits mask indicating which bits of the secret key are injected:\n"
     "                 only these which corresponding mask bit is set (equal to 1) are\n"
     "                 injected\n"
     "    val (long): The 64 bits secret key to inject\n"
     "\n"
     "Returns:\n"
     "    `False` if a conflict was detected between the new bit values and some\n"
     "    previously known ones, `True` if there was no conflict\n\n"},
    {"get_sk",  (PyCFunction)km_get_sk_wrapper, METH_VARARGS,
     "Extract from a key manager the knowledge about a subkey of a round key.\n"
     "\n"
     "Args:\n"
     "    rk (int): The round key number (1 for the first to 16 for the last)\n"
     "    sk (int): The subkey number (1 for the leftmost to 8 for the rightmost)\n"
     "\n"
     "Returns:\n"
     "    `{key, mask}` where key is the subkey as a 6 bits integer, and mask is\n"
     "    another 6 bits integer indicating which bits are known. Every set bit of\n"
     "    the mask indicates a known bit\n\n"},
    {"get_rk",  (PyCFunction)km_get_rk_wrapper, METH_VARARGS,
     "Extract from a key manager the knowledge about a round key.\n"
     "\n"
     "Args:\n"
     "    rk (int): The round key number (1 for the first to 16 for the last)\n"
     "\n"
     "Returns:\n"
     "    `{key, mask}` where key is the roundkey as a 48 bits integer, and mask is\n"
     "    another 48 bits integer indicating which bits are known. Every set bit of\n"
     "    the mask indicates a known bit\n\n"},
    {"get_c0d0",  (PyCFunction)km_get_c0d0_wrapper, METH_VARARGS,
     "Extract from a key manager the knowledge about a 56 bits secret key (after\n"
     "permutated choice 1 - PC1, that is, C0D0 or C16D16 of the DES standard).\n"
     "\n"
     "Returns:\n"
     "    `{key, mask}` where key is the secret key as a 56 bits integer, and mask is\n"
     "    another 56 bits integer indicating which bits are known. Every set bit of\n"
     "    the mask indicates a known bit\n\n"},
    {"get_key",  (PyCFunction)km_get_key_wrapper, METH_VARARGS,
     "Extract from a key manager the knowledge about a 64 bits secret key.\n"
     "\n"
     "Returns:\n"
     "    `{key, mask}` where key is the secret key with parity bits set as\n"
     "    a 64 bits integer, and mask is another 64 bits integer indicating\n"
     "    which bits are known. Every set bit of the mask indicates a known bit\n\n"},
    {"unknown", (PyCFunction)km_unknown, METH_NOARGS,
     "Return an iterator over all the unknown bits of a key manager. It offers\n"
     "a loop scheme over all possible values of a partially known secret key::\n"
     "\n"
     "keym = km.km ()\n"
     "...\n"
     "for key in keym.unknown ():\n"
     "    ...\n"
     "\n"
     "Attention\n"
     "=========\n"
     "\n"
     "The number of iterations exponentially depends on the number of unknown\n"
     "bits. Use with care\n"
     "\n"
     "Returns:\n"
     "    An iterator over all possible values of a partially known secret key\n\n"},
    {NULL}  /* Sentinel */
};

static PyTypeObject km_kmType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "km.km",                   /*tp_name*/
    sizeof(km_kmObject),       /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)km_dealloc,    /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "A key manager object to manage the partial knowledge we have about a 64 bits\n"
    "secret key\n"
    "\n"
    "Attributes:\n"
    "    key (long): The 56 bits word representing C0D0 = C16D16 = PC1(KEY)\n"
    "    mask (long): The 56 bits word in which each set bit indicates a known bit\n\n",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    km_methods,                /* tp_methods */
    km_members,                /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)km_init,         /* tp_init */
    0,                         /* tp_alloc */
    km_new,                    /* tp_new */
};

static PyMethodDef kmMethods[] = {
    {NULL}  /* Sentinel */
};

PyMODINIT_FUNC initkm(void) {
    PyObject* m = Py_InitModule3("km", kmMethods,
               "A data structure and a set of functions to manage the partial knowledge\n"
               "about a DES (Data Encryption Standard) secret key.\n"
               "\n"
               ":Date: 2009-07-08\n"
               ":Authors:\n"
               "    - Renaud Pacalet, renaud.pacalet@telecom-paristech.fr\n\n");

    if (m == NULL)
        return;

    km_kmType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&km_kmType) < 0)
        return;

    if (PyType_Ready(&km_kmIteratorType) < 0)
        return;

    Py_INCREF(&km_kmType);
    Py_INCREF(&km_kmIteratorType);
    PyModule_AddObject(m, "km", (PyObject *)&km_kmType);
    PyModule_AddObject(m, "kmIterator", (PyObject *)&km_kmIteratorType);

    InternalError = PyErr_NewException("km.KMError", NULL, NULL);
    Py_INCREF(InternalError);
    PyModule_AddObject(m, "KMError", InternalError);
}
