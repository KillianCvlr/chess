#include "../include/jeu.hpp"
#include <iostream>
#include <fstream>

int main() {
  Jeu monjeu;
  // On redirige la sortie clog vers un fichier
  ofstream outlog("log.txt");
  auto old_rdbuf = clog.rdbuf();
  clog.rdbuf(outlog.rdbuf());

  // boucle de jeu, s'arrete a la fin de la partie
  bool stop(false);
  do {
      
      monjeu.affiche();
      stop = monjeu.coup();
  } while (!stop);
  // Reset the rdbuf of clog.
  clog.rdbuf(old_rdbuf);

  return 0;
}