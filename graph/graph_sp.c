#include "graph_sp.h"

//Visita in profondità dal solo albero di copertura partente dalla sorgente con applicazione dei vincoli
int *graph_sp_DFS(GRAPHlist grafo_lista, int idx_src, int idx_dst)  {
    int idx;
    int *pred = (int *)malloc(sizeof(int) * (grafo_lista->idx_max)+1);    //per futura utilità, dispongo anche l'array degli indici dei predecessori
    int *dist_dest = (int *)malloc(sizeof(int));    //distanza calcolata a partire dalla sorgente

    char *color = (char *)malloc(sizeof(char) * (grafo_lista->idx_max)+1);   //creo l'array dei colori associati ai vertici, quantificati in grafo_lista[0]

    for(idx=0;idx<=grafo_lista->idx_max;idx++)    {       //inizializzazione grafo
        if(grafo_lista->vrtx[idx] && grafo_lista->vrtx[idx]->adj)
            color[idx] = 'w';
            pred[idx] = -1;
    }
    //N.B.: per poter riconoscere la presenza di (almeno) un percorso che raggiunga la destinazione, non visiterò altre radici bianchi al di fuori della sorgente
    *dist_dest = INT_MAX;
    graph_sp_DFS_visit(grafo_lista->vrtx, idx_src, idx_src, idx_dst, pred, 0, dist_dest, color, 1); //parto solo dalla radice
    
    if(*dist_dest != INT_MAX)
        printf("Percorso TROVATO - Distanza complessiva: %d\n", *dist_dest);
    else
        printf("ATTENZIONE: non è stato trovato alcun percorso\n");
    free(color);
    return pred;
}

//Durante la visita in profondità ricorsiva
int graph_sp_DFS_visit(GRAPHvrtx *vrtx, int idx_curr, int idx_src, int idx_dst, int *pred, int dist_curr, int *dist_dest, char *color, int isAscent)    {
    LIST adj_curr = vrtx[idx_curr]->adj; //prendo gli elementi della lista di adiacenza del vertice attuale
    color[idx_curr] = 'g'; //GRIGIO sul vertice attuale
    int ret = 0, ret_global = 0;
    
    while(adj_curr)    {    //ciclo fin quando non svuoto la Lista
        if(color[adj_curr->idx_vrtx_dst] == 'w'     //se BIANCO
        && graph_sp_conditionElev(vrtx[idx_curr]->height, vrtx[adj_curr->idx_vrtx_dst]->height, isAscent, vrtx[idx_src]->height, vrtx[idx_dst]->height)) {  //se l'adiacente rispetta i vincoli richiesti
            if(adj_curr->idx_vrtx_dst == idx_dst)   {              //se ho raggiunto la destinazione dal nodo attuale                
                if(dist_curr + adj_curr->weight < *dist_dest)  {   //e la distanza accumulata fino alla destinazione è minore della distanza calcolata precedentemente 
                    *dist_dest = dist_curr + adj_curr->weight;     //aggiorno il valore della distanza accumulata fino alla destinazione
                    pred[adj_curr->idx_vrtx_dst] = idx_curr;       //applico l'attuale vertice come predecessore della destinazione
                    color[idx_curr] = 'w';                         //rimetto in BIANCO il nodo attuale in caso di presenza di altri percorsi calcolabili
                    return 1;                                      //non continuo la visita del prossimo adiacente e torno indietro
                }
            } else if((ret = graph_sp_DFS_visit(vrtx, adj_curr->idx_vrtx_dst, idx_src, idx_dst, pred, dist_curr + adj_curr->weight, dist_dest, color, graph_sp_checkIsAscent(vrtx[idx_curr]->height, vrtx[adj_curr->idx_vrtx_dst]->height, isAscent)))) {   //visito il nodo appena incontrato
                //se ret == '1', vuol dire che è stata trovata la destinazione, quindi assegno il predecessore
                pred[adj_curr->idx_vrtx_dst] = idx_curr;       //applico l'attuale vertice come predecessore di questo nodo adiacente   
                ret_global = 1; //e nel R.A. precedente, avviso che questo successore è valido per il percorso
                    //N.B.: quando si visita un altro adiacente dopo che ret == 1, ret potrebbe essere anche == 0 per la visita di altri percorsi,
                    //quindi ret_global mi assicura che questo nodo venga calcolato nel percorso
            }                
        }
        adj_curr = adj_curr->next;  //passo al prossimo vertice adiacente        
    }
    color[idx_curr] = 'w';  //rimetto in BIANCO il nodo per gli altri percorsi da visitare
    if(ret_global)  //se è stata trovata la destinazione da almeno un percorso da questo nodo
        ret = ret_global;
    return ret;
}



//Semplificazione delle condizioni per l'inserimento in coda di elementi adiacenti
//N.B.: per una migliore comprensione, invece di inserire tutte le condizioni in un unico 'if', possiamo analizzare separatamente i casi validi per l'inserimento in coda
int graph_sp_conditionElev(int height_curr, int height_adj, int isAscent, int height_src, int height_dst)  {
    int ret = 0;    //inizializzazione a 0 (nel caso nessuna condizione sia verificata, la funzione ritornerà 0)
    if(height_src == height_curr || height_curr <= height_dst) {   //Se parto dalla sorgente, oppure la destinazione è situata più in alto,
        if(height_curr <= height_adj && isAscent)   {        //devo necessariamente salire o passare all'adiacente posto alla stessa altitudine.
            if(height_adj != height_dst)     //Se salendo non trovo già la destinazione,
                ret = 1;
        }
    } else if(height_curr > height_dst)    {    //Se, invece, mi trovo più in alto rispetto alla destinazione,
        if(isAscent)                            //Se sto salendo (il predecessore è più in basso, oppure salivo dopo aver passato un'intersezione posta allo stesso livello)  
            ret = 1;                            //posso sia salire che scendere, o percorrere l'adiacente posto alla stessa altitudine
        else {                                  //Se sto scendendo (il predecessore è più in alto, oppure scendevo dopo aver passato un'intersezione posta allo stesso livello),
            if(height_curr >= height_adj && height_adj >= height_dst) //devo necessariamente scendere, o percorrere l'adiacente posto alla stessa altitudine, non andando oltre al di sotto del nodo di destinazione
                ret = 1;                            //inserendo solo i nodi adiacenti in discesa.
        }                    
    }
    return ret;
}

//Controllo il passaggio da salita a discesa
int graph_sp_checkIsAscent(int height_curr, int height_adj, int isAscent)   {
    if(isAscent && height_curr > height_adj) //se il successore nella visita va in discesa
        isAscent = 0;               //da ora in poi valgono solo percorsi in discesa
    return isAscent;
}

//Stampa del percorso minimo fra due vertici definito dall'array dei predecessori
void graph_sp_path_print(GRAPHvrtx *vrtx, int idx_src, int idx_dst, int *pred)  {
    if(idx_src == idx_dst)  {
        printf("%d ", idx_src);
    } else    {
        graph_sp_path_print(vrtx, idx_src, pred[idx_dst], pred);
        printf("-> %d ", idx_dst);
    }
}

