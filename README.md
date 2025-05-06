# Detectare Semne de Circulație

Această aplicație detectează automat semnele de circulație dintr-o imagine pe baza **culorii** (roșu/albastru) și a **formei** (triunghi, pătrat, cerc, octogon). Mai jos sunt descriși pașii de implementare:

---

## 1. Încărcarea imaginii și preprocesare

- Imaginea este citită în format BGR folosind `imread`.
- Se aplică un filtru Gauss (`GaussianBlur`) pentru reducerea zgomotului și netezirea marginilor.
- Conversia în HSV se face cu `cvtColor` pentru a facilita detecția robustă a culorii, separând nuanța de luminozitate.

---

## 2. Crearea și rafinarea măștilor de culoare

- Se definesc intervale HSV pentru:
  - **Roșu** – două intervale separate pentru nuanțe deschise și închise.
  - **Albastru** – un singur interval.
- Folosind `inRange`, imaginea este convertită în măști binare.
- Se aplică operații morfologice:
  - `morphologyEx` cu `MORPH_OPEN`, `MORPH_CLOSE`, și `dilate`.

---

## 3. Umplerea găurilor din forme

- Se folosește `floodFill` pentru umplerea fundalului.
- Se aplică `bitwise_not` și combinarea cu masca originală prin `bitwise_or`.

---

## 4. Detecția contururilor

- Contururile sunt extrase cu `findContours`.
- Se filtrează:
  - Cele cu arie prea mică.
  - Cele care se suprapun cu altă culoare.

---

## 5. Determinarea formei geometrice

Fiecare contur este aproximat cu `approxPolyDP`, iar forma este identificată prin:

- **Triunghi** – 3 colțuri (sau 4, dacă un colț este estompat).
- **Pătrat** – 4 colțuri și raport lățime/înălțime ≈ 1.
- **Cerc** – multe colțuri, circularitate mare:
- **Octogon** – 7–9 colțuri, formă echilibrată.

---

## 6. Verificarea culorii în interiorul formei

Funcția `areCuloare`:

1. Creează o mască pe baza conturului.
2. Aplică `inRange` pe mască.
3. Calculează proporția de pixeli validați.
4. Acceptă dacă proporția este > 20%.

---

## 7. Afișarea rezultatelor

Dacă toate criteriile sunt îndeplinite:

- Se desenează un **dreptunghi** cu `rectangle`.
- Se adaugă un **text descriptiv** cu `putText` (ex: „STOP”, „TRIUNGHI ROȘU”).
- Se afișează imaginea originală și imaginea cu semnele detectate.

---

## Exemple posibile rezultate

- `CERC ALBASTRU`
- `TRIUNGHI ROȘU`
- `STOP (Octogon Roșu)`

---

## Tehnologii utilizate

- OpenCV (Python/C++)
- Procesare imagini: HSV, morfologie, detecție contururi

---

## Autor

Proiect realizat ca parte a unui exercițiu de analiză vizuală și recunoaștere a semnelor de circulație.
