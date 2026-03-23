# Activité 4 - SR05

## Projet fait en Standard C

L'idée est simple pour pouvoir séparer le flux d'entrée (stdin) ainsi qu'un timer qui fait émettre un message toutes les X secondes.

Pour se faire j'utilise la fonction "poll" de la librairie standard C (est aussi un syscall Unix) avec 2 descripteurs de fichiers.

Poll va bloquer l'éxécution du programme jusqu'à ce qu'un descripteur de fichier soit readable/writable.

- Le premier est sur stdin, il permet de savoir quand une entrée est faite sur stdin (bloque bien tant qu'il n'y a rien)
- Le second est un timer avec la fonction timerfd_create, permettant d'emettre un event POLLIN qui indique que le timer est fini (il est bien géré en arrière plan du programme (os))

## Références :

- [https://man7.org/linux/man-pages/man2/ppoll.2.html]
- [https://man7.org/linux/man-pages/man2/timerfd_create.2.html]
