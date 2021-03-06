#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

static int trenutni_broj_porcija, broj_divljaka, broj_porcija_za_kuvanje, broj_porcija_po_divljaku;
static pthread_mutex_t m_cinija;
sem_t s_prazan,s_pun;

//funkcija rada divljaka koja se poziva kao nit
void *jedi(void *id){
    //dodeljujemo id divljaka
    int* idDivljaka =(int*) id;
    int x=1;
    //prolazimo kroz petlju onoliko puta koliko porcija zadajemo divljacima
    while(x<=broj_porcija_po_divljaku){
        //zakljucavamo ciniju za ostale divljake, dok trenutni ne pocne da jede
        pthread_mutex_lock(&m_cinija);
        //ako nema porcija u ciniji, divljak budi kuvara, i ceka potvrdu da su obroci skuvani
        if (trenutni_broj_porcija == 0) {printf ("Divljak [%d]: budi kuvara\n", *idDivljaka);sem_post(&s_prazan);sem_wait(&s_pun);}
        //divljak uzima jednu porciju
        trenutni_broj_porcija--;
        printf ("Divljak [%d]: je poceo da jede %d. porciju i ostalo je %d porcije u ciniji\n", *idDivljaka, x, trenutni_broj_porcija);
        //divljak oslobadja ciniju, jer je poceo da jede
        pthread_mutex_unlock(&m_cinija);
        //divljak jede 2 sekunde
        sleep(2);
        printf ("Divljak [%d]: je pojeo %d. porciju\n", *idDivljaka, x);
        x++;
    }    
    printf ("Divljak [%d]: je zavrsio sa jelom\n", *idDivljaka);
}

//funcija rada kuvara koja se poziva kao nit
void *kuvaj () {
        while(1){
            //kuvar ceka da mu divljak javi da je cinija prazna
            sem_wait(&s_prazan);
            printf ("Kuvar je poceo da kuva\n");
            //kuvar kuva 5 sekundi
		    sleep(5);
		    //kuvar dodaje zadati broj porcija
            trenutni_broj_porcija+=broj_porcija_za_kuvanje;
            //kuvar obavestava divljaka da su porcije skuvane
            sem_post(&s_pun);
            printf ("Kuvar je skuvao %d porcije\n", broj_porcija_za_kuvanje);
        }
}


int main(){
    //sa konzole unosimo sledece vrednosti:
    printf("Unesi broj pocetnih porcija M:");
    scanf ("%d",&trenutni_broj_porcija);
    printf("Unesi broj divljaka:");
    scanf ("%d",&broj_divljaka);
    printf("Unesi broj porcija koje kuvar skuva odjednom:");
    scanf ("%d",&broj_porcija_za_kuvanje);
    printf("Unesi broj porcija po divljaku:");
    scanf ("%d",&broj_porcija_po_divljaku);
    
    //niz niti divljaka
    pthread_t niti[broj_divljaka]; 
    
    //niz koji se koristi za dodavanje id-ja divljacima
    int ids[broj_divljaka];
    
    // Inicijalizuje se mutex da osigura medjusobno iskljucenje u grupi divljaka.
    //U ''jedi'' funkciji samo jedan divljak moze uzeti hranu iz cinije u isto vreme
    pthread_mutex_init(&m_cinija, NULL);

    // Inicijalizacija semafora, cija je pocetna vrednost 0
    sem_init(&s_prazan, 0, 0);
    sem_init(&s_pun, 0, 0);
    
    //pokrecemo niti divljaka
    for (int i=0; i<broj_divljaka; i++){
        ids[i]=i+1;
        pthread_create(&niti[i], NULL, jedi, &ids[i]);
    }
    
    //kreiramo i pokrecemo nit kuvara
    pthread_t nitKuvar;
    pthread_create (&nitKuvar, NULL, kuvaj, NULL);
    
    
    //Ceka se da svi zavrse sa jelom
    for (int i=0; i<broj_divljaka; i++) {
        pthread_join(niti[i], NULL);
    }
    
    printf("Svi su jeli i ostalo je %d porcija u ciniji\n", trenutni_broj_porcija);
    return 0;
}
