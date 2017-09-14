
English summary: Network television for finnish YLE-taxpayers.

= Mekkalamooses

Tällä ohjelmistolla voivat suomalaiset YLE-veronmaksajat helpohkosti seurata
(lähes reaaliaikaisesti) ja selailla YLEn tv-ohjelmatarjontaa netistä hiukan
vanhemmillakin (linux, ja mahd. muillakin unix-yhteensopivilla) tietokoneilla.

= Masennus

Joidenkin komponenttien kääntämiseen tarvitaan gtk+2 ja webkitgtk1
(versio 2.4.x). Linux-paketinhallinnassa usein riittää jompikumpi näistä:

    (sudo) dnf install webkitgtk-devel

    (sudo) apt-get install libwebkitgtk-dev

Tämän jälkeen normaali `make` kääntää puuttuvat ohjelmapalikat, jonka jälkeen
tätä voi testata suorittamalla `./mekkalamooses`. `make install` asentaa
tiedostot (useimmin) kotihakemistoon `~/.local/share/`:n taakse. Katso
tulostetta (tai jos unohdat, tee `make install` uudelleen) jos kiinnostaa
tarkasti minne.

Suoratoistovaihtoehdot käyttävät vlc:ta, joten se on asennettava. Yleensä
se on helposti saatavilla kaikille mahdollisille alustoille.

Käytössä saattaa `yle-dl` osoittautua hyödylliseksi. Helpoiten usein riittävän
tuoreen version saa käyttämällä (python2) `virtualenv` -komentoa. Testausta
varten `virtualenv venv` -komennon voi suorittaa nykyisessä työhakemistossa
(eli tässä jossa on myös ./mekkalamooses). Asennusta varten `venv` korvataan
(siis useimmin) polulla `$HOME/.local/share/mekkalamooses/venv`. Tämän
jälkeen suoritetaan

    source /polku/venv/bin/activate
    pip install yle-dl

Huomaa, että myös symbolista linkkiä `venv` -kohdehakemistoon voi käyttää.

`yle-dl` käyttää *AdobeHDS* php-ohjelmaa (tulee uuden yle-dl:n mukana).
`AdobeHDS.php` vaatii php:n että toimii ollenkaan -- ja sitten se valittaa
jos jotain sen tarvitsemia php-paketteja(*) puuttuu. Ainakin Fedora 20:ssa
puuttuvat paketit oli helppo asentaa `yum`:lla joten eiköhän se onnistu
helposti myös muuallakin. AdobeHDS:n toimivuuden voi testata
suorittamalla `php /path/to/AdobeHDS.php`.

((*) koodista katottuna vaatimukset: "bcmath", "curl", "mcrypt" ja "SimpleXML".
Esim. Fedora 20:ssa tämä täyttyy paketeilla `php-bcmath`, `php-common`,
`php-mcrypt` ja `php-xml`...
... Ja näköjäänn Ubuntu 15.10:ssa pittää pakettien `php5-cli`, `php5-curl`,
ja `php5-mcrypt` asennusten jälkeen suorittaa `sudo php5enmod mcrypt`.)

= Tuki

Palautetta ja korjauksia/-ehdotuksia otetaan mielellään vastaan.
Varsinkin niistä asioista jotka ovat jo nyt ongelmallisia.
