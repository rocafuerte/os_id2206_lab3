#include "malloc.h"
#include <stdio.h>

static Header base; /* GLOBAL: Första addressen i free-listan */
static Header *freep = NULL; /* GLOBAL: Nästa lediga plats i free-listan */ 
void *malloc(size_t nbytes){
  Header *p; /* Pekare till nästa lediga minnesarea */
  Header *prevp; /* Pekare till förra lediga minnesarean */
  Header *morecore(size_t); /* Funktion som allocerar mer minne */
  size_t nunits; /* Antal block som efterfrågas */

  nunits = (nbytes+sizeof(Header)-1)/sizeof(Header)+1;
  if((prevp = freep) == NULL){ /* Ingen free-list ännu*/
    base.s.ptr = freep = prevp = &base; /* Alla pekar på &base */
    /*base.s.ptr  = &base;*/ /* Alla pekar på &base */
    base.s.size = 0;
  }else{
    /* prevp=freep alltså*/
  }  
  
  /* p pekar på den föregåendes nästa (sig själv?)
   * Efter varje varv så 
   * pekar prevp på p och
   * p pekar på den p pekade på.
   */  
  fprintf(stderr,"Innan loop\n");
  for(p = prevp->s.ptr; ; prevp = p , p=p->s.ptr){    
    fprintf(stderr,"I loop\n");
    /* Här kan man lägga in strategier */
    if(p->s.size >= nunits){/* om nästa lediga plats har tillräckigt med utrymme */      
      if(p->s.size == nunits){/* Passar precis */
	fprintf(stderr,"Passar precis\n");
	/* ta bort den ur listan */
	prevp->s.ptr = p->s.ptr;
      }else{ /* Det blev plats över */
	fprintf(stderr,"Passar, plats över\n");
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

void free(void *ap){
  Header *bp;
  Header *p;
  bp = (Header *)ap-1; /* För att peka på headern och inte det tomma utrymmet */
  /* Hotta position är den nya platsen skall sättas in */
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

}

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
  free((void *)(up +1)); /*Kör free på den ny vunna platsen, men intepåheadern* för att */
  /*lägga till den i free-listan*/
  return freep;
}
