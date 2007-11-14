#include <Python.h>
#include <nzb_fetch.h>

typedef struct {
    PyObject_HEAD;
    nzb_file *file;
} nzbfetch_NZBFileObject;

typedef struct {
    PyObject_HEAD;
    nzb_fetch *fetcher;
} nzbfetch_NZBFetchObject;






/*
 *
 * NZBFile class
 *
 */
static PyObject *
NZBFile_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    nzbfetch_NZBFileObject *self;

    self = (nzbfetch_NZBFileObject *)type->tp_alloc(type, 0);

    return (PyObject *)self;
}


static void
NZBFile_dealloc(nzbfetch_NZBFileObject* self)
{
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
NZBFile_init(nzbfetch_NZBFileObject* self)
{
    
    return 0;
}


static int
NZBFile_set_storage_path(nzbfetch_NZBFileObject* self, PyObject *args)
{
    char *path;
    
    if (!PyArg_ParseTuple(args, "s", &path))
        return 0;
    
    nzb_fetch_storage_path(self->file, path);
    return Py_None;
}


static int
NZBFile_set_temporary_path(nzbfetch_NZBFileObject* self, PyObject *args)
{
    char *path;
    
    if (!PyArg_ParseTuple(args, "s", &path))
        return 0;
    
    nzb_fetch_temporary_path(self->file, path);
    return Py_None;
}


static PyObject *
NZBFile_list_files(nzbfetch_NZBFileObject* self, PyObject *path)
{
    nzb_file_info **files;
    int num_files;
    PyObject *pylist;
    int i;
    PyObject *item;
    
    num_files = nzb_fetch_list_files(self->file, &files);
    
    pylist = PyList_New(num_files);

    for (i=0; i<num_files; i++) {
      item = Py_BuildValue("{s: i, s: s, s: i}",
                           "id", i,
                           "filename", files[i]->filename,
                           "filesize", 0);
      
      PyList_SetItem(pylist, i, item);
    }
    
    return pylist;
}


PyObject *
NZBFile_create(PyObject *args)
{

}

static PyMethodDef NZBFile_methods[] = {
    {"set_storage_path", (PyCFunctionWithKeywords)NZBFile_set_storage_path,
    METH_KEYWORDS,
     "Set the storage path"
    },
    {"set_temporary_path", (PyCFunctionWithKeywords)NZBFile_set_temporary_path,
    METH_KEYWORDS,
     "Set the temporary path"
    },
    {"list_files", (PyCFunction)NZBFile_list_files,
    METH_NOARGS,
     "Return a list with all files in the NZB file"
    },
    {NULL}  /* Sentinel */
};



PyTypeObject nzbfetch_NZBFileType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "nzbfetch.NZBFile",       /*tp_name*/
    sizeof(nzbfetch_NZBFileObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)NZBFile_dealloc,          /*tp_dealloc*/
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
    "Noddy objects",           /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                      /* tp_iternext */
    NZBFile_methods,          /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)NZBFile_init,      /* tp_init */
    0,                         /* tp_alloc */
    NZBFile_new,                 /* tp_new */
};




/*
 *
 * NZBFetch class
 *
 */

static PyObject *
NZBFetch_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    nzbfetch_NZBFetchObject *self;

    self = (nzbfetch_NZBFetchObject *)type->tp_alloc(type, 0);

    return (PyObject *)self;
}

static void
NZBFetch_dealloc(nzbfetch_NZBFetchObject* self)
{
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
NZBFetch_init(nzbfetch_NZBFetchObject* self)
{
    self->fetcher = nzb_fetch_init();
    
    return 0;
}


static PyObject *
NZBFetch_addserver(nzbfetch_NZBFetchObject* self, PyObject *args, PyObject *kwds)
{
    PyObject *hostname, *port, *username, *password, *connections, *priority;
    
    static char *kwlist[] = {"hostname", "port", "username", "password",
                             "connections", "priority", NULL};
                             
    
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "sissii", kwlist, 
                                      &hostname, &port, &username, &password,
                                      &connections, &priority))
        return NULL;
    

    nzb_fetch_add_server(self->fetcher, hostname, port,
                         username, password, connections, priority);
    
    return Py_None;
    

};

static int
NZBFetch_download(nzbfetch_NZBFetchObject* self, PyObject *args)
{
    int id;
    PyObject *file;
    nzb_file_info **files;
    int num_files;
    
    if (!PyArg_ParseTuple(args, "O!i", &nzbfetch_NZBFileType, &file, &id))
        return 0;
    
    
    num_files = nzb_fetch_list_files(((nzbfetch_NZBFileObject *)file)->file, &files);
    nzb_fetch_download(self->fetcher, files[id]);
    
    return Py_None;
}

static PyObject *
NZBFetch_connect(nzbfetch_NZBFetchObject* self)
{
    nzb_fetch_connect(self->fetcher);
    return Py_None;
};


static PyMethodDef NZBFetch_methods[] = {
    {"addserver", (PyCFunctionWithKeywords)NZBFetch_addserver,
    METH_VARARGS|METH_KEYWORDS,
     "Add a server"
    },
    {"download", (PyCFunction)NZBFetch_download,
    METH_VARARGS,
     "Download given file"
    },
    {"connect", (PyCFunctionWithKeywords)NZBFetch_connect, METH_NOARGS,
     "Connect to the specified servers"
    },    
    {NULL}  /* Sentinel */
};



PyTypeObject nzbfetch_NZBFetchType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "nzbfetch.NZBFetch",       /*tp_name*/
    sizeof(nzbfetch_NZBFetchObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)NZBFetch_dealloc,          /*tp_dealloc*/
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
    "Noddy objects",           /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                      /* tp_iternext */
    NZBFetch_methods,          /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)NZBFetch_init,      /* tp_init */
    0,                         /* tp_alloc */
    NZBFetch_new,                 /* tp_new */
};


/*
 *
 * INIT
 *
 */

/* nzbfetch methods */
static PyObject * nzbfetch_version()
{
    return Py_BuildValue("s", "versie 1");
}

static PyObject * nzbfetch_parse(PyObject *self, PyObject *args)
{
    PyTypeObject *type = &nzbfetch_NZBFileType;
    nzbfetch_NZBFileObject *obj;
    
    char *fn;
    
    if (!PyArg_ParseTuple(args, "s", &fn))
        return 0;
    
    
    obj = (nzbfetch_NZBFileObject* )type->tp_alloc(type, 0);
    NZBFile_init(obj);

    
    obj->file = nzb_fetch_parse(fn);

    return obj;
}


static PyMethodDef NZBFetchMethods[] = {
    //{"version",  nzbfetch_version, METH_VARARGS, "Execute a shell command."},
    {"parse",  nzbfetch_parse, METH_VARARGS, "Parse the nzb file"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initnzbfetch(void)
{
    PyObject* m;


    m = Py_InitModule3("nzbfetch", NZBFetchMethods,
                       "Example module that creates an extension type.");

    nzbfetch_NZBFetchType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&nzbfetch_NZBFetchType) < 0)
        return;


    nzbfetch_NZBFileType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&nzbfetch_NZBFileType) < 0)
        return;


    //Py_INCREF(&nzbfetch_NZBFetchType);
    PyModule_AddObject(m, "NZBFetch", (PyObject *)&nzbfetch_NZBFetchType);
    PyModule_AddObject(m, "NZBFile", (PyObject *)&nzbfetch_NZBFileType);
}

