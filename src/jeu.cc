#include "../include/jeu.hpp"
#include "../include/utilitaires.hpp"


Jeu::Jeu ()
    : echiquier(new Echiquier(true)), couleur_joueur(Blanc)  {
        clog << "La Partie a été créée !" << endl;
    }

Jeu::~Jeu () {}

void Jeu::setDernierMouv(string move){
    this->dernier_mouvement = move;
}

string Jeu::getDernierMouv() {
    return this->dernier_mouvement;
}

void Jeu::setJoueur(couleur_t c){
    this->couleur_joueur = c;
}

couleur_t Jeu::getJoueur(){
    return this->couleur_joueur;
}

void Jeu::affiche() {echiquier->affiche();}

bool Jeu::passage_possible(Pos posDeb, Pos posFin){
    // Pas de test pour le L du cavalier car il passe au dessus des pièces
    int x0 = posDeb.getX();
    int y0 = posDeb.getY();
    int xFinal = posFin.getX();
    int yFinal = posFin.getY();

    // horizontale
    if (x0 == xFinal){
        int x = x0;
        if (y0 < yFinal){
            for (int i = y0 + 1 ; i < yFinal ; ++i) {
                if (this->echiquier->getPiece(Pos(x, i)) != nullptr){
                    return false;
                }
            }
        }else{
            for (int i = yFinal + 1 ; i < y0 ; ++i) {
                if (this->echiquier->getPiece(Pos(x, i)) != nullptr){
                    return false;
                }
            }
        }
    }

    // verticale
    if (y0 == yFinal){
        int y = y0;
        if (x0 < xFinal){
            for (int i = x0 + 1 ; i < xFinal ; ++i) {
                if (this->echiquier->getPiece(Pos(i, y)) != nullptr){
                    return false;
                }
            }
        }else{
            for (int i = xFinal + 1 ; i < x0 ; ++i) {
                if (this->echiquier->getPiece(Pos(i, y)) != nullptr){
                    return false;
                }
            }
        }
    }

    // diagonale
    if (abs(x0 - xFinal) == abs(y0 - yFinal)){
        int x = x0;
        int y = y0;
        if (x0 < xFinal){
            if (y0 < yFinal){
                for (int i = x0 + 1 ; i < xFinal ; ++i) {
                    ++x;
                    ++y;
                    if (this->echiquier->getPiece(Pos(x, y)) != nullptr){
                        return false;
                    }
                }
            }else{
                for (int i = x0 + 1 ; i < xFinal ; ++i) {
                    ++x;
                    --y;
                    if (this->echiquier->getPiece(Pos(x, y)) != nullptr){
                        return false;
                    }
                }
            }
        }else{
            if (y0 < yFinal){
                for (int i = xFinal + 1 ; i < x0 ; ++i) {
                    --x;
                    ++y;
                    if (this->echiquier->getPiece(Pos(x, y)) != nullptr){
                        return false;
                    }
                }
            }else{
                for (int i = xFinal + 1 ; i < x0 ; ++i) {
                    --x;
                    --y;
                    if (this->echiquier->getPiece(Pos(x, y)) != nullptr){
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

void Jeu::affichage_fin_jeu(string resultat) {
    clog << this->echiquier->canonicalPosition() << " " << resultat << endl;
}

bool Jeu::est_capturable(Pos pos, couleur_t c){
    Square dest = *this->echiquier->getSquare(pos);
    for (int i = 0 ; i < TAILLE_PLATEAU ; ++i){
        for (int j = 0 ; j < TAILLE_PLATEAU ; ++j){
            Pos pos_ij(i, j);
            Piece *piece = this->echiquier->getPiece(pos_ij);
            if (piece != nullptr && piece->getCouleur() != c){
                if (strcmp(typeid(*piece).name() + 1, "Cavalier") == 0){
                    if (piece->mouvement_legal(dest)){
                        return true;
                    }
                }else{
                    if (piece->mouvement_legal(dest, true)
                        && this->passage_possible(pos_ij, dest)
                    ){
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool Jeu::echec_au_roi(couleur_t c){
    Square* posRoi = this->echiquier->position_roi(c);
    if (posRoi == nullptr){
        cerr << "Le roi n'a pas ete trouve" << endl;
        exit(1);
    }

    return this->est_capturable(*posRoi, c);
}

bool Jeu::met_en_echec(Pos posDeb, Pos posFin ,couleur_t c){
    Square *CaseDeb = echiquier->getSquare(posDeb);
    Square *CaseFin = echiquier->getSquare(posFin);
    Piece *pieceDepart = CaseDeb->getPiece();
    Piece *pieceFin = CaseFin->getPiece();
    bool echec;
    //Deplacement au niveau de l'échiquier (pour tester l'échec)
    echiquier->deplace_piece(CaseDeb, CaseFin);
    //On effectue le test
    echec = echec_au_roi(c);

    //On repositionne les pièces dans les deux cas
    echiquier->pose_piece(pieceDepart,CaseDeb);
    echiquier->pose_piece(pieceFin, CaseFin);
    if (pieceDepart != nullptr){
        pieceDepart->setSquare(CaseDeb);
    }
    if (pieceFin != nullptr){
        pieceFin->setSquare(CaseFin);
    }
    return echec;
}

bool Jeu::prise_en_passant(Pos posDeb, Pos posFin){
    //On regarde si le dernier mouvmeent était un mouvement classique
    if (getDernierMouv().empty() || !entree_mouvement(getDernierMouv())){
        return false;
    }
    Pos debutDernierMouv = Pos(getDernierMouv().substr(0,2));
    Pos finDernierMouv = Pos(getDernierMouv().substr(2,2));
    Piece *dernierePieceJouee = echiquier->getPiece(finDernierMouv);

    // On regarde si la dernière pièce jouée est un pion
    if ((dernierePieceJouee != nullptr 
            && strcmp(typeid(*dernierePieceJouee).name()+1, "Pion") == 0)
        // Si son mouvement était bien celui attendu (Noir ou Blanc)
        && (debutDernierMouv.getX() == finDernierMouv.getX() + 2
                || debutDernierMouv.getX() == finDernierMouv.getX() - 2)
        // Et si le mouvement actuel est bien une prise en passant correspondante
        && (dernierePieceJouee->getSquare()->getY() == posFin.getY()
            && abs(posDeb.getX() - posFin.getX()) == abs(posDeb.getY() - posFin.getY())
            && abs(finDernierMouv.getX() - posFin.getX()) != 0
                    ))
                    return true;
    return false;
}

bool Jeu::petit_roque(couleur_t c) {
    if (c == Blanc){
        if(this->passage_possible(Pos("e1"), Pos("h1" ))
            && this->echiquier->getPiece(Pos("e1"))->getNbDeplacement() == 0
            && this->echiquier->getPiece(Pos("h1"))->getNbDeplacement() == 0
            && this->echiquier->getPiece(Pos("e1")) != nullptr
            && this->echiquier->getPiece(Pos("h1")) != nullptr){
            if(!this->deplace_piece(Pos("e1"), Pos("g1"), true)){
                clog << "Movement du roi impossible" << endl;
                return false;
            }
            this->deplace_piece(Pos("h1"), Pos("f1"), true);
            return true;
        }
    }else{
        if(this->passage_possible(Pos("e8"), Pos("h8" ))
            && this->echiquier->getPiece(Pos("e8"))->getNbDeplacement() == 0
            && this->echiquier->getPiece(Pos("h8"))->getNbDeplacement() == 0
            && this->echiquier->getPiece(Pos("e8")) != nullptr
            && this->echiquier->getPiece(Pos("h8")) != nullptr){
            if(!this->deplace_piece(Pos("e8"), Pos("g8"), true)){
                clog << "Mouvement du roi impossible" << endl;
                return false;
            }
            this->deplace_piece(Pos("h8"), Pos("f8"), true);
            return true;
        }
    }

    clog << "Petit roque impossible" << endl;
    return false;
}

bool Jeu::grand_roque(couleur_t c) {
    if (c == Blanc){
        if(this->passage_possible(Pos("e1"), Pos("a1" ))
            && this->echiquier->getPiece(Pos("e1"))->getNbDeplacement() == 0
            && this->echiquier->getPiece(Pos("a1"))->getNbDeplacement() == 0
            && this->echiquier->getPiece(Pos("e1")) != nullptr
            && this->echiquier->getPiece(Pos("a1")) != nullptr){
            if(!this->deplace_piece(Pos("e1"), Pos("c1"), true)){
                clog << "Mouvement du roi impossible !" << endl;
                return false;
            }
            this->deplace_piece(Pos("a1"), Pos("d1"), true);
            return true;
        }
    }else{
        if(this->passage_possible(Pos("e8"), Pos("a8" ))
            && this->echiquier->getPiece(Pos("e8"))->getNbDeplacement() == 0
            && this->echiquier->getPiece(Pos("a8"))->getNbDeplacement() == 0
            && this->echiquier->getPiece(Pos("e8")) != nullptr
            && this->echiquier->getPiece(Pos("a8")) != nullptr){
            if(!this->deplace_piece(Pos("e8"), Pos("c8"), true)){
                clog << "Mouvement du roi impossible !" << endl;
                return false;
            }
            this->deplace_piece(Pos("a8"), Pos("d8"), true);
            return true;
        }
    }

    clog << "Grand roque impossible" << endl;
    return false;
}

bool Jeu::promotion_possible(Pos posFin){
    Square* caseFin = this->echiquier->getSquare(posFin);
    Piece *pieceJouee = caseFin->getPiece();
    const char *NomDeClasse = typeid(*pieceJouee).name() + 1;
    couleur_t couleur = pieceJouee->getCouleur();
    int ligneCase = caseFin->getX();
    //On teste que la piece soit un pion et qu'il soit bien placé
    if (strcmp(NomDeClasse, "Pion") == 0
        && ((couleur == Noir && ligneCase == 0)
            || (couleur == Blanc && ligneCase == 7)))
                return true;
    return false;
}

bool Jeu::deplace_piece(Pos posDeb, Pos posFin, bool continueJeu) {
    posDeb.affiche_xy() ;
    posFin.affiche_xy();
    Piece *pieceEnDeplacement = echiquier->getPiece(posDeb);
    Piece *pieceADestination = echiquier->getPiece(posFin);
    Square *CaseDeb = echiquier->getSquare(posDeb);
    Square *CaseFin = echiquier->getSquare(posFin);
    if (pieceEnDeplacement == nullptr){
        return false;
    }

    if (pieceEnDeplacement->getCouleur() != couleur_joueur){
        return false;
    }

    // Cas spécial : prise en passant
    const char *NomDeClasse = typeid(*pieceEnDeplacement).name() + 1;
    if (strcmp(NomDeClasse, "Pion") == 0){
        if (this->prise_en_passant(posDeb, posFin)){
            Square* casePionCapture = this->echiquier->getSquare(Pos(getDernierMouv().substr(2,2)));
            Piece* pionCapture = casePionCapture->getPiece();
            this->echiquier->pose_piece(nullptr, casePionCapture );
            if(met_en_echec(posDeb, posFin, couleur_joueur)){
                this->echiquier->pose_piece(pionCapture, casePionCapture );
                return false;
            }
            if(!continueJeu){
                this->echiquier->pose_piece(pionCapture, casePionCapture );
                return true;
            }
            pionCapture->set_etat(Capture);
            this->echiquier->deplace_piece(CaseDeb, CaseFin);
            pieceEnDeplacement->incr_nb_deplacement();
            return true;
        }
    }

    //Validité du coup
    if(!(strcmp(NomDeClasse, "Roi") == 0 )){
        if (pieceADestination != nullptr){
            if (pieceADestination->getCouleur() != pieceEnDeplacement->getCouleur()){
                if (strcmp(NomDeClasse, "Pion") == 0){
                    if (!pieceEnDeplacement->mouvement_legal(*echiquier->getSquare(posFin), true)) {
                        clog << "Mouvement invalide !" << endl;
                        return false;
                    }
                }else{
                    if (!pieceEnDeplacement->mouvement_legal(*echiquier->getSquare(posFin))) {
                        clog << "Mouvement invalide !" << endl;
                        return false;
                    }
                }
            } else{
                clog << "La case de destination est dejà occupée par une de vos pièce !" << endl;
                return false;
            }
        }else{
            if (!pieceEnDeplacement->mouvement_legal(*echiquier->getSquare(posFin))) {
                clog << "Mouvement invalide !" << endl;
                return false;
            }
        }
    }
    clog << "Verification de la validité du coup passée !" << endl;
    //Passage libre
    if ((strcmp(NomDeClasse, "Pion") == 0
        || strcmp(NomDeClasse, "Roi") == 0
        || strcmp(NomDeClasse, "Dame") == 0
        || strcmp(NomDeClasse, "Fou") == 0
        || strcmp(NomDeClasse, "Tour") == 0)){
        if (!this->passage_possible(posDeb, posFin)){
            clog << "Passage impossible !" << endl;
            return false;
        }
    }
    clog << "Passage libre !" << endl;
    // Le coup met en echec son roi
    if (this->met_en_echec(posDeb, posFin ,this->getJoueur())){
        clog << "Ce coup vous met en échec !" << endl;
        return false;
    }
    clog << "Ne met pas en echec !" << endl;

    //On s'arrete là pour les test, pas de déplacement
    if(!continueJeu) return true;

    //Déplacement de la pièce
    this->echiquier->deplace_piece(CaseDeb, CaseFin);
    pieceEnDeplacement->incr_nb_deplacement();
    clog << "Piece déplacée !" << endl;

    //Vérifie si le coup amène un pion à la promotion
    if (this->promotion_possible(posFin)){
        clog << "Promotion disponible !\n Attente de la promotion (Q(ueen) R(ook) B(ishop) K(night)) :" << endl;

        string prom_input;
        getline(cin, prom_input);

        while (!promotion_valide(prom_input)){
            clog << "L'input n'est pas valide !" << endl;
            clog << "Attente de la promotion (Q(ueen) R(ook) B(ishop) K(night)) :" << endl;
            getline(cin, prom_input);
        }

        this->echiquier->promotion(pieceEnDeplacement, prom_input);

    }
    return true;
}

bool Jeu::coup() {
    
    string input;
    bool stop = false;
    bool echec = false;

    clog << "C'est au joueur " << 
            ((couleur_joueur == Blanc) ? "blanc " : "noir ") 
            << "de jouer" << endl;

    do {
        do {
            if (echec_au_roi(this->couleur_joueur)){
                clog << "Echec !"  << endl;
                echec = true;
            }
            if (mat(couleur_joueur)){
                clog << "Mat !" << endl;
                if (echec){
                    if (this->getJoueur() == Noir){
                        this->affichage_fin_jeu("0-1");
                    }else{
                        this->affichage_fin_jeu("1-0");
                    }
                    return true;
                }
                this->affichage_fin_jeu("1/2-1/2");
                return true;
            }
            clog << "Attente du coup :  ";
            getline(cin, input);

            while (!entree_valide(input)) {
                clog << "L'entrée n'est pas valide !" << endl;
                clog << "Attente du coup :  ";
                getline(cin, input);
            }
            //Les commandes faisant s'arreter la partie
            if (entree_sortie(input)){
                clog << "quit" << endl;
                this->affichage_fin_jeu("?-?");
                return true;

            } else if (entree_egalite(input)){
                clog << "draw" << endl;
                this->affichage_fin_jeu("1/2-1/2");
                return true;

            } else if (entree_abandon(input)) {
                clog << "resign" << endl;
                if (this->getJoueur() == Blanc){
                    this->affichage_fin_jeu("0-1");
                }else{
                    this->affichage_fin_jeu("1-0");
                }
                return true;
            //Les commandes classiques du jeu

            }else if (entree_show(input)){
                this->affiche();
            }else if (entree_petit_rock(input)){
                clog << "petit roque" << endl;
                stop = this->petit_roque(this->couleur_joueur);

            }else if (entree_grand_rock(input)){
                clog << "grand roque" << endl;
                stop = this->grand_roque(this->couleur_joueur);

            }else{
                clog << "mouvement classique" << endl;
                stop = this->deplace_piece((Pos(input.substr(0,2))),
                                       Pos(input.substr(2,2))
                );
            }
        }while(this->echec_au_roi(this->couleur_joueur));

    } while (!stop);
    //On garde le dernier mouvement en memoire
    this->setDernierMouv(input);
    //Changement de joueur
    this->couleur_joueur = (this->couleur_joueur == Blanc) ? Noir : Blanc;
    
    //Tout se passe bien donc on n'arrete pas la partie
    return false;
}

bool Jeu::mat(couleur_t c){
    for (auto p : this->echiquier->getPieces(c))
    {
        if (p->getEtat() == EnJeu && this->enleve_echec(p))
        {
            return false;
        }
    }
    return true;
}


bool Jeu::enleve_echec(Piece *pieceTest){
    Square* caseDeb = pieceTest->getSquare();
    Pos posDeb = Pos(caseDeb->getX(), caseDeb->getY());

    for (int i = 0; i < TAILLE_PLATEAU; i++)
    {
        for (int j = 0; j < TAILLE_PLATEAU; j++)
        {
            Pos pos_ij = Pos(i,j);
            if (deplace_piece(posDeb, pos_ij, false))
            {
                return true;
            }      
        }
    }
    return false;
}