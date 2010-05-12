#include "malloc.h"
#include <stdio.h>
#define NALLOC 1024
#define QUICKLISTELEMENTS 16 /* Antal element en ny quicklist */
#define SMALLESTQUICKLIST 8
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
static Header *quicklist[NRQUICKLISTS+1]={0}; /* En lista med 5 quicklists plus 1 vanlig */


static Header *quickfit_morecore(size_t bytes){
    char *cp;
    char *it;
    int i=0;
    cp = sbrk((sizeof(Header)+bytes)*QUICKLISTELEMENTS);
    if(cp == (char *) -1 ){/* Gör koden portabel */
        return NULL; /* Om det inte fanns disk */
    }
        
    for(it=cp;i<QUICKLISTELEMENTS;it+=(sizeof(Header)+bytes) , i++){
        ((Header*) it)->s.size = bytes;         
        ((Header*) it)->s.ptr = (Header *)it+(sizeof(Header)+bytes);
    }
    ((Header*) it)->s.ptr = NULL; /* Sista pekar på NULL */
    return (Header *)cp;
}
/* nu är i enhet Headers */
static Header *morecore(size_t nu){
    char *cp;
    /*char *sbrk(int);  Definierar att vi kommer att använda den eller?*/
    Header *up;
  
    if(nu < NALLOC){ /* Även om man frågar efter lite så blir den minst NALLOC */
        nu = NALLOC;
    }
    cp = sbrk(nu * sizeof(Header)); /* Fråga efer mer utrymme */
    if(cp == (char *) -1 ){/* Gör koden portabel */
        return NULL; /* Om det inte fanns disk */
    }
    up = (Header *) cp; /* Castar det nua utrymmer till en Header*/
    up->s.size = nu; /* Sätter storleken på det nya utrymmet till det man frågade efter*/
    fprintf(stderr,"Lägger till %d units i free listan.\n",nu);
    free((void *)(up +1)); /*Kör free på den ny vunna platsen, men intepåheadern* för att */
    /*lägga till den i free-listan*/
    return freep;
}

void *malloc(size_t nbytes){
    fprintf(stderr,"malloc\n");
    if(nbytes <= 0) return NULL;
    Header *p; /* Pekare till nästa lediga minnesarea */
    Header *prevp; /* Pekare till förra lediga minnesarean */
    Header *morecore(size_t); /* Funktion som allocerar mer minne */
    size_t nunits; /* Antal block som efterfrågas */
    Header * bestp = NULL; /* hålla reda på bäst hittills i bestfit */
    Header * best_prevp = NULL; /* För att kunna plocka ur freelistan i bestfit */
    nunits = (nbytes+sizeof(Header)-1)/sizeof(Header)+1;
    if(STRATEGY<4){
        if((prevp = freep) == NULL){ /* Ingen free-list ännu*/
            fprintf(stderr,"Freelistan tom\n");
            base.s.ptr = freep = prevp = &base; /* Alla pekar på &base */
            base.s.size = 0;
        }
    }
  
    if(STRATEGY==1){
        for(p = prevp->s.ptr; ; prevp = p , p=p->s.ptr){    
            fprintf(stderr,"I loop\n");
            if(p->s.size >= nunits){/* om nästa lediga plats har tillräckigt med utrymme */
                if(p->s.size == nunits){
                    fprintf(stderr,"Passar precis\n");
                    /* ta bort den ur listan */
                    prevp->s.ptr = p->s.ptr;
                }else{ /* Det blev plats över */
                    fprintf(stderr,"Passar, plats över\n");
                    fprintf(stderr,"Tar bort %d units från freelistan\n",nunits);                
                    p->s.size -= nunits; /* minska antalet platser som finns kvar */
                    p += p->s.size; /* Pekar-aritmetik. Flyttar fram den lediga positionen. */
                    p->s.size = nunits;
                }
                freep = prevp; /* peka om nästa lediga plats till den föregående */
                return (void *)(p+1);/* Returnera +1 för att få adressen till platsen och inte headern */
            }
            if(p==freep){ /* Fanns ingen plats */
                fprintf(stderr,"Fanns ingen plats.\n");
                if((p = morecore(nunits)) == NULL){ /* Om all disk är slut */
                    return NULL;
                }
            }    
        }
    }
    else if(STRATEGY==2){
        /* Taktik: gå igenom hela freelistan och leta efter den plats som är "minst för stor" */
        /* Hittar vi en perfect match så bryter vi loopen */
        for(p = prevp->s.ptr; ; prevp = p , p=p->s.ptr){    
            fprintf(stderr,"I loop\n");
            if(p->s.size >= nunits){/* om nästa lediga plats har tillräckigt med utrymme */      
                if(p->s.size == nunits){/* Passar precis */
                    fprintf(stderr,"Passar precis. Smutt!\n");
                    /* ta bort den ur listan */
                    prevp->s.ptr = p->s.ptr;
                    freep = prevp; /* peka om nästa lediga plats till den föregående */
                    return (void *)(p+1);/* Returnera +1 för att få adressen till platsen och inte headern */    
                }else{ /* Det blev plats över */
                    fprintf(stderr,"Passar, plats över\n");
                    /* är det en fin plats? */
                    if (NULL == bestp || (p->s.size - nunits) < (bestp->s.size - nunits)) { /* Inte hittat nån eller hittat en bättre! */
                        bestp = p;
                        best_prevp = prevp;            
                    }
                }
            }
            if (freep == p) { /* Gått igenom hela listan? */
                if (NULL == bestp) { /* hittade ingen plats som duger */
                    /* Fanns ingen plats */
                    fprintf(stderr,"Fanns ingen plats.\n");
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
 
    else if(STRATEGY==3){
        /* Taktik: gå igenom hela freelistan och leta efter den plats som är sämst! */
        /* (Ska man ändå bryta om man hittar en perfekt?) */
        for(p = prevp->s.ptr; ; prevp = p , p=p->s.ptr){    
            fprintf(stderr,"I loop\n");
            if(p->s.size >= nunits){/* om nästa lediga plats har tillräckigt med utrymme */
                fprintf(stderr,"Passar, plats över. Ändrar bestp\n");
                if(p->s.size == nunits){/* Passar precis */
                    fprintf(stderr,"Passar precis. Alltså smutt!\n");
                    /* ta bort den ur listan */
                    prevp->s.ptr = p->s.ptr;
                    freep = prevp; /* peka om nästa lediga plats till den föregående */
                    return (void *)(p+1);/* Returnera +1 för att få adressen till platsen och inte headern */
                }else{ /* Det blev plats över */
                    if (NULL == bestp || (p->s.size - nunits) > (bestp->s.size - nunits)) { /* Inte hittat nån än eller hittat en bättre! */
                        bestp = p;
                        best_prevp = prevp;
                    }
                }
            }
         
            if (freep == p) { /* Gått igenom hela listan? */
                if (NULL == bestp) { /* hittade ingen plats som duger */
                    fprintf(stderr,"Fanns ingen plats.\n");
                    if((p = morecore(nunits)) == NULL){ /* Om all disk är slut */
                        return NULL;
                    }
                } else { /* Vi har hittat en trevlig plats med lite mer än krävd plats */
                    fprintf(stderr,"Tar bort %d units från freelistan\n",nunits);
                    bestp->s.size -= nunits; /* minska antalet platser som finns kvar */
                    bestp += bestp->s.size; /* Pekar-aritmetik. Flyttar fram den lediga positionen. */
                    bestp->s.size = nunits;
                    freep = best_prevp; /* peka om nästa lediga plats till den föregående */
                    return (void *)(bestp+1);/* Returnera +1 för att få adressen till platsen och inte headern */
                }    
            }
        }
    }
    /* Quick fit */
    else if(STRATEGY == 4){
        /* Vilken freelista */
        int list_place = 0;
        int f_list_size;
        if(nbytes > SMALLESTQUICKLIST * (2^ (NRQUICKLISTS-1))){
            list_place=NRQUICKLISTS;
        }else{ 
            for (f_list_size=SMALLESTQUICKLIST;
                 nbytes > f_list_size ;
                 ++list_place, f_list_size *=2) {
            }
        }
        printf("listplace: %d, innehåll: %d\n", list_place, quicklist[list_place]);

        if(quicklist[list_place]==NULL){
            /* Gör ny lista */
            if (list_place < NRQUICKLISTS-1) { /* ej sista listan, som implementeras med vanlig first-fit */
                if((quicklist[list_place] = quickfit_morecore(SMALLESTQUICKLIST*(2^list_place))) == NULL){ 
                    /* Om all disk är slut */
                    return NULL;
                }
            } else { /* sista listan, allokera endast exakt det som krävs */
                quicklist[list_place] = sbrk(sizeof(Header)+nbytes);
                if(quicklist[list_place] == (char *) -1 ){/* Gör koden portabel */
                    quicklist[list_place]=NULL;
                    return NULL; /* Om det inte fanns disk */
                }   
                return quicklist[list_place];
            }            
        }else{ /* Fanns plats */
            Header * ptr = quicklist[list_place];
            quicklist[list_place] = (quicklist[list_place]->s.ptr)->s.ptr;
            return ptr;
        }      
    }
}


void free(void *ap){
    if(ap == NULL) return;
    Header *bp;
    Header *p;
    bp = (Header *)ap-1; /* För att peka på headern och inte det tomma utrymmet */
    if(bp->s.size < 0){
        fprintf("försöker freea en header som har size negativt: %d\n",bp->s.size);
        return;
    }
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
    fprintf(stderr,"realloc\n");
    free(ptr); /* Freear innan för att kunna återanvända minnet om den nya storleken får plats */
    void * p = memcpy(malloc(size),ptr,size);
    return p;
}

