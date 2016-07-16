#ifndef _AK8963_H_
#define _AK8963_H_

class FAK8963{
    public:
        FAK8963(void);
        ~FAK8963(void);
        void Test(void);

    private: 
        unsigned char st1;
        unsigned char  st2;
        float val[3];

};


#endif

