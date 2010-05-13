#include "malloc.h"
#include <stdio.h>
#define NALLOC 1024
#define QUICKLISTELEMENTS 4 /* Antal element en ny quicklist */
#define SMALLESTQUICKLIST 8
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

static Header *quicklist[NRQUICKLISTS]={0}; /* En lista med QUICKLISTELEMENTS headers */

static Header *quickfit_morecore(size_t nu){
    printf("I quickfit_morecore, units = %d\n", nu);
    char *cp;
    char *it;
    int i=0;
    cp = sbrk(nu*((sizeof(Header)+1))*QUICKLISTELEMENTS);
    if(cp == (char *) -1 ){/* Gör koden portabel */
        return NULL; /* Om det inte fanns disk */
    }
    for(it=cp;i<QUICKLISTELEMENTS;it+=(nu*sizeof(Header)+sizeof(Header)), i++){
        /*((Header*) it)->s.size = nu*sizeof(Header);*/ /* blir bytes */
        ((Header*) it)->s.size = nu; /* Antal units */
        ((Header*) it)->s.ptr = it+((nu+1)*sizeof(Header));
        fprintf(stderr,"pekare %d= %d, pekar på %d\n",i,it,( (Header *)it)->s.ptr);   
    }
    fprintf(stderr,"efter loop\n");
    it -= (nu+1)*sizeof(Header);
    ((Header*) it)->s.ptr = NULL; /* Sista pekar på NULL */
    fprintf(stderr,"pekare %d= %d, pekar på %d\n",--i,it,( (Header *)it)->s.ptr);
    return (Header *)cp;
}
/* nu är i enhet Headers */
static Header *morecore(size_t nu){
    fprintf(stderr,"morecore(%d)\n",nu);
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
    fprintf(stderr,"morecore returnerear(freep): %d\n",freep);
    return freep;
}

void *malloc(size_t nbytes){
    fprintf(stderr,"malloc %d bytes\n",nbytes);
    if(nbytes <= 0) return NULL;
    Header *p; /* Pekare till nästa lediga minnesarea */
    Header *prevp; /* Pekare till förra lediga minnesarean */
    Header *morecore(size_t); /* Funktion som allocerar mer minne */
    size_t nunits; /* Antal block som efterfrågas */
    Header * bestp = NULL; /* hålla reda på bäst hittills i bestfit */
    Header * best_prevp = NULL; /* För att kunna plocka ur freelistan i bestfit */
    nunits = (nbytes+sizeof(Header)-1)/sizeof(Header)+1;
    if((prevp = freep) == NULL){ /* Ingen free-list ännu*/
        fprintf(stderr,"Freelistan tom\n");
        base.s.ptr = freep = prevp = &base; /* Alla pekar på &base */
        base.s.size = 0;
    }
    
    /* Vilken freelista */
    int list_place = 0;
    int f_list_size;
    if(4==STRATEGY && nbytes > SMALLESTQUICKLIST * (1<<(NRQUICKLISTS-1))){
        list_place=NRQUICKLISTS;
    }else{ 
        /* Hitta lista */
        for (f_list_size=SMALLESTQUICKLIST;
             nbytes > f_list_size ;
             ++list_place, f_list_size *=2) {
        }
    }
    
    if(STRATEGY == 4 && quicklist[list_place]==NULL){
        /* Gör ny lista */
        printf("Listan tom, gör ny!\n");
        if (list_place < NRQUICKLISTS-1) { /* ej sista listan, som implementeras med vanlig first-fit */
            printf("listplace: %d, innehåll: %d\n", list_place, quicklist[list_place]);
            printf("quickfit_morecore(%d)\n", nunits);
                /*quicklist[list_place] = quickfit_morecore(SMALLESTQUICKLIST*(1<<(list_place)));*/
            quicklist[list_place] = quickfit_morecore(nunits);
            printf("efter morcore\n");
            if (quicklist[list_place] == NULL){                     
                /* Om all disk är slut */
                return NULL;
            }
            /* länka om till nästa objekt, returnera det första */ 
        } 
    }
    /* Fanns plats, eller i sista listan */
    if (STRATEGY == 4 && list_place < NRQUICKLISTS -1) {
        printf("Fanns plats, plockar ut den lilla platsen typ...\n");
        Header * ptr = quicklist[list_place];
        fprintf(stderr,"Första objektet: %d, det den pekar på: %d\n ",quicklist[list_place], quicklist[list_place]->s.ptr);   
        quicklist[list_place] = (quicklist[list_place]->s.ptr);        
        fprintf(stderr,"Vi returnerar adress: %d\n", ptr+1);
        return ptr+1;
        /* TODO */
    } else {/* stora firstfit listan */
        for(p = prevp->s.ptr; ; prevp = p , p=p->s.ptr){    
            fprintf(stderr,"I loop, p = %d\n",p);
            if(p->s.size >= nunits){/* om nästa lediga plats har tillräckigt med utrymme */
                fprintf(stderr,"I loop, får plats här\n");
                if(p->s.size == nunits){
                    fprintf(stderr,"Passar precis\n");
                    /* ta bort den ur listan */
                    prevp->s.ptr = p->s.ptr;
                    freep = prevp; /* peka om nästa lediga plats till den föregående. */
                    return (void *) (p+1); /* returnera +1 för att få adress till datat, inte headern...*/
                }else{ /* Det blev plats över */
                    fprintf(stderr,"Passar, plats över\n");
                    fprintf(stderr,"Tar bort %d units från freelistan\n",nunits);                
                    if (STRATEGY == 1) {
                        p->s.size -= nunits; /* minska antalet platser som finns kvar */
                        p += p->s.size; /* Pekar-aritmetik. Flyttar fram den lediga positionen. */
                        p->s.size = nunits;
                        freep = prevp; /* peka om nästa lediga plats till den föregående */
                        
                        return (void *)(p+1);/* Returnera +1 för att få adressen till platsen och inte headern */
                    }
                    if (STRATEGY == 2) {
                        fprintf(stderr,"Kör strategi 2: passar, plats över\n");
                        /* är det en fin plats? */
                        if (NULL == bestp || (p->s.size - nunits) < (bestp->s.size - nunits)) { /* Inte hittat nån eller hittat en bättre! */
                            bestp = p;
                            best_prevp = prevp;            
                        }
                    }
                    if (STRATEGY == 3) {
                        fprintf(stderr,"Kör strategi 3: passar, plats över\n");
                        /* är det en fin plats? */
                        if (NULL == bestp || (p->s.size - nunits) > (bestp->s.size - nunits)) { /* Inte hittat nån eller hittat en "sämre"! */
                            bestp = p;
                            best_prevp = prevp;            
                        }
                    }                    
                }
            }
            if(p==freep){ /* Gått igenom hela listan */               
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
}

void print_free_lists(){
    Header * ap;
    int i;
    for(i=0;i<NRQUICKLISTS;i++){
        fprintf(stderr,"\nLista %d \n",i);
        if(quicklist[i]==NULL){
            fprintf(stderr,"NULL\n");
            continue;
        }
        ap=quicklist[i];        
        while(ap!=NULL){
            fprintf(stderr,"Element %d pekar på: %d. Size: %d\n",ap,ap->s.ptr,ap->s.size);
            ap = ap->s.ptr;
        }
    }
    
    fprintf(stderr,"\nVanliga freelistan\n");
    ap=freep->s.ptr;
    fprintf(stderr,"&base: %d\n",&base);
    while(ap!=NULL){
        fprintf(stderr,"Element %d pekar på: %d. Size: %d\n",ap,ap->s.ptr,ap->s.size);
        ap = ap->s.ptr;
        if(ap == &base){
            fprintf(stderr,"Kom till base\n");
            break;
        }
    }    
}

void free(void *ap){
    fprintf(stderr,"FREEEEE\n");
    if(ap == NULL) return;
    Header *bp;
    Header *p;
    bp = (Header *)ap-1; /* För att peka på headern och inte det tomma utrymmet */
    int list_place = 0;
    int f_list_size;
    
    if(bp->s.size < 0){
        fprintf(stderr,"försöker freea en header som har size negativt: %d\n",bp->s.size);
        return;
    }
    fprintf(stderr,"******bp->s.size: %d\n",bp->s.size);    
    /* Om det är någon av quicklistorna. OBS size antas vara antal Headers.*/
    
    
    if(bp->s.size < (SMALLESTQUICKLIST * (1<<(NRQUICKLISTS-1)))) {
        fprintf(stderr,"%d ska frias i lill-listan\n",bp); 
        /* hitta lista */
        for (f_list_size=SMALLESTQUICKLIST;
             (bp->s.size-1)*sizeof(Header) > f_list_size ;
             ++list_place, f_list_size *=2) {
        }
        
        fprintf(stderr,"Free i lista: %d\n",list_place);
        bp->s.ptr = quicklist[list_place];
        quicklist[list_place] = bp;
        return;  
    }
    fprintf(stderr,"%d ska frias i stor-listan\n",bp); 
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
    fprintf(stderr,"realloc\n");
    if(ptr==NULL) return malloc(size); 
    if(ptr!=NULL && 0 == size) {    
        free(ptr);    
        return NULL;  
    }
    
    fprintf(stderr,"malloc\n");    
    void * p = malloc(size);
    fprintf(stderr,"(((Header *)ptr)-1)->s.size:%d \n",(((Header *)ptr)-1)->s.size);
    fprintf(stderr,"(((Header *)ptr)-1)->s.size*sizeof(Header):%d \n",(((Header *)ptr)-1)->s.size*sizeof(Header));
    fprintf(stderr,"(((Header *)p)-1)->s.size*sizeof(Header):%d \n",(((Header *)p)-1)->s.size*sizeof(Header));
    if(size < (((Header *)ptr)-1)->s.size*sizeof(Header)){
        fprintf(stderr,"memcpy\n");
        memcpy(p, ptr,size);
    }else{
        fprintf(stderr,"memcpy\n");
        memcpy(p, ptr, (((Header *)(ptr)-1)->s.size-1)*sizeof(Header));
    }
    fprintf(stderr,"free\n");    
    free(ptr);
    return p;
}

