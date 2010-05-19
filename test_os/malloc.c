#include "malloc.h"
#include <stdio.h>
#define NALLOC 1024
#define QUICKLISTELEMENTS 8 /* Antal element en ny quicklist */
#define SMALLESTQUICKLIST 8 /* Units */
/* Om NRQUICKLISTS == 0 så får vi STRATEGY 1-3 */ 
typedef long Align; /* Ändra till rätt */

union header{
    struct{
        union header *ptr;
        size_t size; 
    } s;
    Align x;
};

typedef union header Header;

static Header base; /* GLOBAL: Första addressen i free-listan */
static Header *freep = NULL; /* GLOBAL: Nästa lediga plats i free-listan */ 

#ifndef NRQUICKLISTS
#define NRQUICKLISTS 0
#endif

static Header* quicklist[NRQUICKLISTS]={0}; /* En lista med QUICKLISTELEMENTS headers */

static Header *quickfit_morecore(size_t nu){
    /*printf("I quickfit_morecore, units = 0x%x\n", nu);*/
    char *cp;
    char *it;
    int i=0;
    cp = sbrk(nu*sizeof(Header)*QUICKLISTELEMENTS);
    if(cp == (char *) -1 ){/* Gör koden portabel */
        return NULL; /* Om det inte fanns disk */
    }
    for(it=cp;i<QUICKLISTELEMENTS;it+=nu*sizeof(Header), i++){
        ((Header*) it)->s.size = nu; /* Antal units */
        ((Header*) it)->s.ptr = it+(nu*sizeof(Header));
    }
    it -= nu*sizeof(Header);
    ((Header*) it)->s.ptr = NULL; /* Sista pekar på NULL */    
    return (Header *)cp;
}
/* nu är i enhet Headers */
static Header *morecore(size_t nu){    
    char *cp;
    /*char *sbrk(int);  Definierar att vi kommer att använda den eller?*/
    Header *up;
  
    if(nu < NALLOC && STRATEGY != 4){ /* Även om man frågar efter lite så blir den minst NALLOC */
        nu = NALLOC;
    }
    cp = sbrk(nu * sizeof(Header)); /* Fråga efer mer utrymme */
    if(cp == (char *) -1 ){/* Gör koden portabel */
        return NULL; /* Om det inte fanns disk */
    }
    up = (Header *) cp; /* Castar det nua utrymmer till en Header*/
    up->s.size = nu; /* Sätter storleken på det nya utrymmet till det man frågade efter*/
    /* fprintf(stderr,"Lägger till 0x%x units i free listan.\n",nu); */
    free((void *)(up +1)); /*Kör free på den ny vunna platsen, men intepåheadern* för att */
    /*lägga till den i free-listan*/
    /*print_free_lists();*/
    return freep;
}

int get_ql_index(int bytes){
    int index=0;
    int f_list_size=0;
    for (f_list_size=SMALLESTQUICKLIST;
         bytes > f_list_size ;     
         ++index, f_list_size *=2) {
    }    
    return index;
}

void *malloc(size_t nbytes){
    /*fprintf(stderr,"malloc %d bytes\n",nbytes);*/
    if(nbytes <= 0) return NULL;
    Header *p; /* Pekare till nästa lediga minnesarea */
    Header *prevp; /* Pekare till förra lediga minnesarean */
    Header *morecore(size_t); /* Funktion som allocerar mer minne */
    size_t nunits; /* Antal block som efterfrågas */
    Header * bestp = NULL; /* hålla reda på bäst hittills i bestfit */
    Header * best_prevp = NULL; /* För att kunna plocka ur freelistan i bestfit */
    int list_place = 0;
    
    nunits = (nbytes+sizeof(Header)-1)/sizeof(Header)+1;
    if((prevp = freep) == NULL){ /* Ingen free-list ännu*/
        base.s.ptr = freep = prevp = &base; /* Alla pekar på &base */
        base.s.size = 0;
    }
    /*fprintf(stderr,"malloc %d units\n",nunits);*/
    /* Vilken freelista */
    if(4==STRATEGY && nbytes > SMALLESTQUICKLIST * (1<<(NRQUICKLISTS-1))){
        list_place=NRQUICKLISTS;
        /*fprintf(stderr,"%d list_place");*/
    }else{         
        /* Hitta lista */
        list_place=get_ql_index(nbytes);
    }

    /*fprintf(stderr,"hör till lista %d \n",list_place);*/
    
    if(STRATEGY == 4 &&        
       list_place < NRQUICKLISTS &&
       quicklist[list_place]==NULL){
        
        /* Gör ny lista */
        /* ej sista listan, som implementeras med vanlig first-fit */
        /*printf("listplace: 0x%x, innehåll: 0x%x\n", list_place, quicklist[list_place]);
          printf("quickfit_morecore(0x%x)\n", nunits);*/
        /* HÄR ÄR DET EN SKILLNAD */
        quicklist[list_place] = quickfit_morecore(nunits);
        if (quicklist[list_place] == NULL){
            return NULL;
        }
        /* länka om till nästa objekt, returnera det första */
    }
    /* Fanns plats, eller i sista listan */
    if (STRATEGY == 4 && list_place < NRQUICKLISTS) {
        Header * ptr = quicklist[list_place]; /* base för quicklist */
        quicklist[list_place] = (quicklist[list_place]->s.ptr);
        /*fprintf(stderr,"quicklist[list_place]: 0x%x quicklist[list_place]->s.ptr: 0x%x\n",quicklist[list_place] ,quicklist[list_place]->s.ptr);*/
        
        /* kan bli fel */        
        /* Sätter en tom lista till NULL för annars fastnar den inte
         *  i if satsen ovan som bestämmer om en ny lista måste göras
         */
        /*
        if(ptr->s.ptr == NULL){
            quicklist[list_place] = NULL;
        }        
        */
        return ptr+1;
        /* TODO */
    } else {/* stora firstfit listan */
        
        for(p = prevp->s.ptr; ; prevp = p , p=p->s.ptr){ 
            if(p->s.size >= nunits){/* SIGSEG!  om nästa lediga plats har tillräckigt med utrymme */
                if(p->s.size == nunits){
                    /*fprintf(stderr,"Passar precis\n");*/
                    /* ta bort den ur listan */
                    prevp->s.ptr = p->s.ptr;
                    freep = prevp; /* peka om nästa lediga plats till den föregående. */
                    return (void *) (p+1); /* returnera +1 för att få adress till datat, inte headern...*/
                }else{ /* Det blev plats över */
                    /*
                    fprintf(stderr,"Passar, plats över\n");
                    fprintf(stderr,"Tar bort 0x%x units från freelistan\n",nunits);
                    */
                    if (STRATEGY == 1 || STRATEGY == 4) {
                        p->s.size -= nunits; /* minska antalet platser som finns kvar */
                        /*fprintf(stderr,"p->s.size: %d, nunits: %d\n", p->s.size,nunits);*/
                        p += p->s.size; /* Pekar-aritmetik. Flyttar fram den lediga positionen. */
                        /*fprintf(stderr,"p: 0x%x, denna pekar på ngt konstigtn &base: 0x%x\n", p,&base);*/
                        p->s.size = nunits; /*SIGSEG */
                        freep = prevp; /* peka om nästa lediga plats till den föregående */  
                        return (void *)(p+1);/* Returnera +1 för att få adressen till platsen och inte headern */
                    }
                    if (STRATEGY == 2) {
                        /*
                        fprintf(stderr,"Kör strategi 2: passar, plats över\n");
                        */
                        /* är det en fin plats? */
                        if (NULL == bestp || (p->s.size - nunits) < (bestp->s.size - nunits)) { /* Inte hittat nån eller hittat en bättre! */
                            bestp = p;
                            best_prevp = prevp;            
                        }
                    }
                    if (STRATEGY == 3) {
                        /*fprintf(stderr,"Kör strategi 3: passar, plats över\n");*/
                        /* är det en fin plats? */
                        if (NULL == bestp || (p->s.size - nunits) > (bestp->s.size - nunits)) { /* Inte hittat nån eller hittat en "sämre"! */
                            bestp = p;
                            best_prevp = prevp;            
                        }
                    }                    
                }
            }
            if(p==freep){ /* Gått igenom hela listan */
                /*fprintf(stderr,"Gått igenom hela listan. p=0x%x, freep: 0x%x\n",p,freep);*/
                if (NULL == bestp) { /* hittade ingen plats som duger */
                    /* Fanns ingen plats */
                    /*fprintf(stderr,"Fanns ingen plats. Kör morcore(%d)\n",nunits);         */
                    if((p = morecore(nunits)) == NULL){ /* Om all disk är slut */
                        return NULL;
                    } 
                } else { /* Vi har hittat en trevlig plats med lite mer än krävd plats */
                    bestp->s.size -= nunits; /* minska antalet platser som finns kvar */
                    bestp += bestp->s.size; /* Pekar-aritmetik. Flyttar fram den lediga positionen. */
                    bestp->s.size = nunits;
                    freep = best_prevp; /* peka om nästa lediga plats till den föregående */
                    return (void *)(bestp+1);/* Returnera +1 för att få adressen till platsen och inte headern */
                }
            }             
        }
    }
}

void print_free_lists(){
    Header * ap;
    int i;
    for(i=0;i<NRQUICKLISTS;i++){
        /*fprintf(stderr,"\nLista %d \n",i);*/
        if(quicklist[i]==NULL){
            /*fprintf(stderr,"NULL\n");*/
            continue;
        }
        ap=quicklist[i];
        fprintf(stderr,"QL[%d] address: 0x%x\n",i,quicklist[i]);
        while(ap!=NULL){
            fprintf(stderr,"QL[%d] - Element 0x%x pekar på: 0x%x. Size: %d\n",i,ap,ap->s.ptr,ap->s.size);
            ap = ap->s.ptr;
        }
    }
    
    fprintf(stderr,"\nVanliga freelistan\n"); 
    ap = freep;
    fprintf(stderr,"&base: 0x%x\n",&base);
    while(ap!=NULL){
        fprintf(stderr,"L - Element 0x%x pekar på: 0x%x. Size: %d\n",ap,ap->s.ptr,ap->s.size);
        ap = ap->s.ptr;
        if(ap == &base){
            break;
        }
    }    
}

void free(void *ap){    
    if(ap == NULL) return;
    Header *bp;
    Header *p;
    bp = (Header *)ap-1; /* För att peka på headern och inte det tomma utrymmet */
    int list_place = 0;
    int f_list_size;
    
    if(bp->s.size < 0){
        fprintf(stderr,"försöker freea en header som har size negativt: 0x%x\n",bp->s.size);
        return;
    }
    if(bp->s.size == 0){
        fprintf(stderr,"försöker freea en header som har size 0! 0x%x\n",bp->s.size);
        return;
    }

    /* Om det är någon av quicklistorna. OBS size antas vara antal Headers.*/        
    if(STRATEGY==4){
        if((bp->s.size - 1)*SMALLESTQUICKLIST < (SMALLESTQUICKLIST * (1<<(NRQUICKLISTS-1)))) {
            /*fprintf(stderr,"Adress 0x%x ska frias i lill-listan\n",bp);  
              fprintf(stderr,"%d units ska frias i lill-listan\n",bp->s.size);*/
            /* hitta lista */ 
            list_place = get_ql_index((bp->s.size-1)*sizeof(Header));
            /*fprintf(stderr,"Free i lista: %d\n",list_place); */
            bp->s.ptr = quicklist[list_place];
            quicklist[list_place] = bp;
            return;
        }
    }
    /*fprintf(stderr,"Adress 0x%x ska frias i stor-listan\n",bp);  
      fprintf(stderr,"%d units ska frias i stor-listan\n",bp->s.size);  */
    /* Hitta position är den nya platsen skall sättas in */
    for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr){
        if(p >= p->s.ptr && (bp > p || bp < p->s.ptr)){
            break;
        }
    }
    
    if(bp + bp->s.size == p->s.ptr){ /* Om granne med en övre ledig, Joina! */
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    }else{
        bp->s.ptr = p->s.ptr;
    }
    if(p + p->s.size == bp){ /*Om den är granne med en undre ledig, Joina!*/
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    }else{
        p->s.ptr = bp;
    }
    freep = p;  
}

void *realloc(void *ptr, size_t  size){        
    if(ptr==NULL) return malloc(size); 
    if(ptr!=NULL && 0 == size) {    
        free(ptr);    
        return NULL;  
    }
    
    /*fprintf(stderr,"realloc ptr: 0x%x, ptr->s.size: %d, ny size: %d\n",ptr,((Header*)ptr-1)->s.size,size);*/
    void * p = malloc(size);
    if (NULL==p) return NULL;
    if(size < ((((Header *)ptr)-1)->s.size-1)*sizeof(Header)){
        memcpy(p, ptr,size);
    }else{
        memcpy(p, ptr, (((Header *)(ptr)-1)->s.size-1)*sizeof(Header));
    }
    free(ptr); 
    return p;
}

