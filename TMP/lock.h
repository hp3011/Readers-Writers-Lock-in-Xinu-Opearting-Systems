#define READ 1
#define WRITE 2

#define LFREE 0
#define LASSIGN 1
#define LACQUIRE 1

#define NLOCKS 50

struct lentry {
    int lstatus;
    int lavail;
    int ltype;
    int lprior;
    int rdr_count;
    int lhold[NPROC];

    //Queue
    int rdr_qhead;
    int rdr_qtail;
    int wrt_qhead;
    int wrt_qtail;
};

extern struct lentry ltab[];