# TP N°1 de compression

## Remarques préliminaires
  * un sujet sera donné par semaine. 
  * le temps du TP ne suffit pas pour réaliser le sujet. Vous devez aussi travaillier chez vous.
  * le travail réalisé doit être rendu dans la semaine qui suit le TP (sinon, note pour ce TP = 0).
  * un sujet d'une semaine suivante peut dépendre d'un sujet précédente. Par exemple, une fonction qui calcule les fréquences des symboles sur un bloc de texte. 

##  Introduction

Afin d'effectuer de la compression, on doit être en mesure d'écrire les données sous la forme la plus compacte possible. En particulier, on souhaite pouvoir écrire paquet de bits par paquets de bits dans un flux de bits, chacun de ces paquets pouvant éventuellement avoir chacun une taille différente. 

Le flux de bits obtenu représente alors l'information compressée. Inversement, lorsque l'on relit ce flux de bits, on doit être en mesure de lire bit à bit, ou paquet par paquet. 
  
On se propose dans ce premier TP de vous familiariser avec la bibliothèque **BitStream** qui permet de manipuler des flux de bits, afin d'effectuer des entrées/sorties simples ces flux.  

## 1. Lecture/écriture en binaire

Dans un premier temps, il vous est demandé de comprendre l'utilisation de cette bibliothèque, et exécutant et en comprenant le code d'exemple fourni. Puis, écrire de petits programmes de test qui vous permettent:  
1. De tester l'écriture puis la lecture bit à bit dans un flux.
2. De tester l'écriture puis la lecture de Bits::Block de taille constante.

L'objectif de cette bibliothèque est de pouvoir générer un flux binaire de taille quelconque à partir de la concaténation de blocs binaires de taille variable ou de bits. Par exemple, si on effectue un codage à taille fixe de 5 bits, l'écriture de n caractères dans ce flux doit produire ceil(5*n/8) octets. De même, l'écriture bit à bit de n bits dans le flux doit produire ceil(n/8) octets.

## 2. Codage binaire à taille fixe (CTF)
On veut dans cette partie produire un codage à taille fixe d'un fichier texte.
1. Afin de tester des données, on considère les fichiers de données contenus dans [ce fichier texte contenant la constitution américaine](USconstitution.txt). Ecrire une fonction permettant permettant de lire les données dans ces fichiers, et de construire la table des caractères différents dans ce fichier.
2. Ecrire une fonction de codage à taille fixe avec le plus petit code binaire possible pour ce fichier. 
3. Ecrire une fonction de décodage permettant de décoder les données codées avec la fonction précédente.
4. Ecrire une fonction permettant de vérifier que les données avant et après codage/décodage sont identiques.
5. Sauvegarder le résultat dans un fichier, dont vous construirez le format (typiquement entête de description + données). Le résultat en terme de taille devra être conforme à ce qui a été indiqué à la question 1 dans la section donnée du fichier. Le format de fichier adopté devra être décrit dans [la déclaration de travaux](declarationTP01.md).
6. Charger le fichier compressé (avec relecture de l'entête), puis décompressez les données contenues.

## Contraintes en terme de programmation

  * le langage de programmation utilisé sera le C++. L'utilisation des fonctions C standard reste autorisé (memcpy, sprintf, ...).
  * chaque méthode de codage/décodage devra être implémentée dans une classe particulière (i.e. une classe CTF, CTV, CHuffman, CLZH).
  * toutes ces classes dériveront d'une classe mère qui implémente les méthodes virtuelles encode et decode.
  * tous les résultats des compressions seront stockés dans des fichiers partageant tous le même format, à savoir une entête contenant la taille du fichier, un magic number pour identifier le type de compression, et dans le cas du CTF, par exemple, le nombre de symboles à lire, la taille de la table des symboles/codes, puis la table des symboles/des codes. Cette entête sera suivie des données écrits en binaire.

## Remise de ce TP
Le TP doit être remis au plus tard le vendredi de la semaine qui suit la date du TP.

Le dépôt s'effectuera à travers l'outil "GitHub Classroom" en utilisant la procédure qui vous a été transmise par courriel.

Sous peine de **voir votre note divisée par deux**, vous devez remplir [la déclaration de travaux](declarationTP01.md) de votre dépôt afin de décrire les travaux réalisés.
___
Année universitaire 2019-2020 : version du 8 janvier 2020.

