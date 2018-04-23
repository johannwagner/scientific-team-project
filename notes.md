## Topic:
 - Intel TBB reicht nicht aus, early and late materalization in DB systems,
 - Instruments als profiler stürzt aufgrund zu vieler threads ab
 - Scan über z.B. 5 GB, Aufteilen der Daten in Bereiche, mergen danach. 
 - Operationsübergreifende Threads, fixe Anzahl im Pool
 - Anzahl dynamisch zur Laufzeit, Struct mit initialisierungsfunktion, nummer an threads zur Laufzeit rübergeben
 - Minimumanforderung round robin, queue drum rum
 - Gegenüber bisherigem Verfahren vergleichen, overhead klein halten
 - Andere Scheduler Algorithmen ausprobieren
 - Ziel: Wieder debuggen können!
 
## Schedule:
 - Milestones, Termine beachten und was hinraten
 
## Team:
 - Aufteilung, Arbeitspakete
 
## Literatur:
 - Basis Thread Pools, Dann details und danach scheduling verfahren

## Präsentation: 
 - Präsentationen 3 tage vorher zusenden
 
## System:
 - dockerhub/geckodb 
 - Bitbucket privates repo
 - Graphbasiert

## Code:
 - Bolster.h -> filter, for, Threadpool mit rübergeben als neues Argument, Prüfen auf Gültigkeit usw
 - Testing Framework aufsetzen, Filter und Threadpool reinkippen und Ergebnisse definieren, Bolster importieren
 