---
title: "HW06 Slovníkový útok na heslo v MD5"
layout: "homework"
list-of-files: ["*.c", "*.h", "upřesnění v zadání"]
publish: now
deadline-early: ~
deadline-final: 2019-06-18 24:00
authors:
  - Filip Procházka: filip@prochazka.su
editors:
  - xmikusin
---

Doplnění zadání

* _zatím nic_

## Představení úkolu

[MD5](https://en.wikipedia.org/wiki/MD5) je [hašovací
funkce](https://en.wikipedia.org/wiki/Cryptographic_hash_function), nebo-li
[jednosměrná šifra](https://en.wikipedia.org/wiki/Cryptographic_hash_function).
Haše se mimo jiné používají, protože není bezpečné ukládat uživatelská hesla do
databází v otevřeném textu. Hned si vysvětlíme proč.

Mějme nějakou webovou aplikaci, do které se lze registrovat, a někdo v ní najde
bezpečnostní zranitelnost. Pomocí této zranitelnosti je schopný získat kompletní
obsah databáze. Toto není žádné sci-fi, ale poměrně běžný jev, viz
[Have I been pwned?](https://haveibeenpwned.com/).

Již tohle je velký problém, ovšem ještě větším problémem jsou uživatelé, kteří
nemají unikátní hesla pro služby, které využívají, jen proto, že je těžké si
zapamatovat spoustu hesel. (Pokud nepoužíváte správce hesel, koukněte na
[LastPass](https://lastpass.com/) nebo
[1Password](https://agilebits.com/onepassword)). Problém je zřejmý, pokud máte
všude stejné heslo, stačí napadnout jednu aplikaci a útočník se může přihlásit
do všech ostatních, klidně i do vašeho e-mailu nebo internetového bankovnictví.

Bezpečnost není vlastnost, ale míra. Není reálné mít dokonale zabezpečenou
aplikaci, ale je možné se tomu limitně blížit. Jednou z věcí, kterou je
bezpodmínečně nutné udělat, je neukládat hesla přímo, ale jen jejich haše. Při
přihlášení se vždy heslo, které uživatel zadal, zahašuje a porovná s hašem
uloženým v databázi. Pokud se haše rovnají, je heslo _nejspíš_ správné.

Pokud nám tedy někdo napadne aplikaci a získá haše hesel, nebude schopný získat
originální hesla. A tedy nebude schopný se přihlásit ani do dalších služeb, u
kterých je uživatel registrovaný a kde má stejné heslo.

MD5 se dnes již nepoužívá, jelikož v ní byla [objevena spousta zranitelností a
nedostatků](https://en.wikipedia.org/wiki/MD5#Security). Ovšem nejnaivnější
způsob, jak získat heslo z MD5 haše je útok hrubou silou. Na moderních
procesorech jde počítat velice rychle, a tedy nám nevadí hloupě vyzkoušet
spoustu vstupů a haše porovnat.

Na internetu bohužel existuje stále spousta článků s návody, jak ukládat hesla
pomocí MD5 (nebo podobně slabé hašovací funkce), jimiž se bohužel spousta
programátorů stále bezmyšlenkovitě řídí. Dále existuje nezanedbatelné množství
softwaru, který ještě nebyl aktualizován, aby využíval modernější hašovací
funkce. Doporučované algoritmy na hašování hesel před uložením jsou
[Argon2](https://en.wikipedia.org/wiki/Argon2),
[Bcrypt](https://en.wikipedia.org/wiki/Bcrypt),
[scrypt](https://en.wikipedia.org/wiki/Scrypt) a
[PBKDF2](https://en.wikipedia.org/wiki/PBKDF2). Pokud zvolíte jeden ze dvou
prvních jmenovaných, rozhodně neuděláte chybu. Tyto algoritmy mají nastavitelnou
rychlost a vytvářejí tak „dražší haše“, které se pak počítají mnohem déle.

Představte si tedy, že jste se dostali k [databázovému výpisu (database
dump)](https://en.wikipedia.org/wiki/Database_dump) s hesly uživatelů a jste
zlým hackerem. Chcete se dostat k originálním heslům. Jedna možnost je, že by se
hesla generovala náhodně, ale mnohem efektivnější je nejprve použít slovníkový
útok.

## Zadání

Napište program, který načte ze souboru seznam slov, která by někdo mohl použít
jako hesla. Tato hesla vyzkoušíte, a následně budete generovat zkomolené
varianty těchto hesel, například `auto` -> `@ut0`, které také vyzkoušíte.

Stáhněte si kostru projektu, která obsahuje:

* Seznam slov vyextrahovaný z [GNU Aspell slovníku](http://aspell.net/).
  Obsahuje pouze slova bez diakritiky, znaky mimo ASCII se testovat nebudou.
* Funkce na spočítání MD5 haše.

Protože slovníky jsou opravdu velké, testujte své řešení nejdříve na menším
souboru s třeba 10 slovy. Až bude program fungovat, zkuste ho spustit na větších
slovnících.

### Spouštění

```
./cracker [OPTIONS] FILE HASH
```

Jediným přepínačem (kromě bonusu) je `-t`, který zapne generování záměn ve
slovech. Argument `FILE` specifikuje název souboru se slovníkem. Argument `HASH`
je 32 znakové hexadecimální číslo reprezentující MD5 hash, ke kterému se má
hledat heslo.

Příklad spuštění:

```
./cracker -t CZ_no_diacricits.txt b8310c07c107099f71ee87cd96837779
```

Pokud program najde shodu, vypíše na standardní výstup na dva řádky:

```
password found
{PASSWORD}
```

kde místo `{PASSWORD}` bude řetězec s nalezeným heslem. Pokud nenajde shodu,
vypíše na jeden řádek:

```
password not found
```

To, že program heslo ve slovníku nenajde, **není chybový stav** a skončí s
nulovou návratovou hodnotou.

### Záměny

Pokud bude zadaný přepínač `-t`, program vyzkouší na každém slovu ze slovníku i
záměny podle následující tabulky. Je nezbytné, aby program uměl generovat
*všechny možné kombinace*.

|  písmeno  |  záměna  |
| :-------: | :------: |
|    `a`    |   `@`    |
|    `a`    |   `4`    |
|    `b`    |   `8`    |
|    `e`    |   `3`    |
|    `i`    |   `!`    |
|    `l`    |   `1`    |
|    `o`    |   `0`    |
|    `s`    |   `$`    |
|    `s`    |   `5`    |
|    `t`    |   `7`    |

Počítejte také s tím, že některá slova jsou „jména“, a tedy začínají velkým
písmenem. Je tedy potřeba identicky nahrazovat i velká písmena.

### Validace argumentů

Pokud program bude zavolán se špatnými argumenty nebo dojde k nějaké chybě,
vypíše na `stderr` vhodnou chybovou hlášku a ukončí se s nenulovou návratovou
hodnotou. Pár tipů, co určitě musíte kontrolovat:

* špatný počet argumentů,
* neznámý přepínač,
* soubor nejde otevřít,
* string neodpovídá korektnímu MD5 haši (ten má vždy 32 znaků).

### Časová a paměťová náročnost

Je požadováno, aby program byl co nejrychlejší, a tedy bude nastaven tvrdý limit
na dobu zpracování. Když si spustíte hledání MD5 haše posledního slova z velkého
slovníku bez transformací na Aise, měl by program doběhnout nejpozději do 20
vteřin, jinak ani nemá smysl zkoušet odevzdávat naostro. Aby testy zbytečně
neumíraly kvůli přetížené Aise, je v testech malá časová rezerva, ale
nespoléhejte se na ni.

Poskytnutý slovník má cca 80 MB. Bylo by sice možné ho načíst celý do paměti a
zpracovat, ale tento přístup přestane fungovat v momentě, kdy například budete
programovat cracker využívající [rainbow tabulku](https://en.wikipedia.org/wiki/Rainbow_table),
protože ty můžou mít klidně terabyty, nebo pokud se slovník do paměti prostě
nevejde. Proto se požaduje, aby paměťová náročnost programu nepřekročila
nezbytnou míru.

[Valgrind](http://valgrind.org/docs/manual/mc-manual.html) je váš kamarád, používejte
ho. Je nezbytné, aby váš program před koncem uvolnil všechny alokované prostředky.

HINT: celý úkol lze napsat bez jediného použití funkce `malloc()`.

### Odevzdávané soubory

Rozvržení řešení do souborů je čistě na vás, nicméně soubory `md5.h` a `md5.c`
**neupravujte**. Kontr při kompilaci vždy použije verzi, která je v kostře.

## Bonusové rozšíření

Implementujte přepínač `-c`, který bude zapínat generování variant z velkých a malých písmen.

Tedy například `aBc` vyzkouší (ne nezbytně v tomto pořadí) všechny následující varianty:

* `abc`
* `abC`
* `aBc`
* `aBC`
* `Abc`
* `AbC`
* `ABc`
* `ABC`

Přepínač `-c` musí fungovat správně v kombinaci s `-t` i samostatně. Přepínače
budou vždycky zadané jako samostatné argumenty, zda bude vaše řešení povolovat i
kombinace `-ct` nebo `-tc` je na vás (ty se testovat nebudou).

## Poznámky

* Alokace jsou „pomalé“, obzvláště pokud jich máte udělat 6,5 milionu. Zkuste
  dělat co nejméně alokací i na zásobníku.
* Při kompilaci pro ladění použijte přepínač `-Og`, pro běh na test rychlosti
  kompilujte s přepínačem `-O3`.
* Můžete používat rozšíření POSIX.1-2008. Vaše řešení bude přeloženo s
  přepínačem `-D_POSIX_C_SOURCE=200809L`. Úkol však lze vyřešit i bez použití
  POSIXu.
* Pro vlastní testování si můžete na Aise zjistit haš libovolného řetězce
  příkazem `echo -n 'ŘETĚZEC' | md5sum`.
* Vzorová implementace je dostupná na Aise v `/home/kontr/pb071/hw06/md5crack`.
