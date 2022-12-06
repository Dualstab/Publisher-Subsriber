# IKP-PS-G16 Publisher - Subscriber
Projekat iz predmeta industrijski komunikacioni protokoli. 

Zadatak:
Napraviti dva PubSub servisa koji mogu da opsluzuju proizvoljan broj klijenata. Servisi treba medjusobno da komuniciraju tako da jedan servis samo prima poruke od Publisher-a a zatim posalje poruke drugom PubSub servisu. Drugi servis prima poruke od prvog PubSub servisa i zatim poruke prosledjuje Subscriber-ima. Interface PubSub servisa je:
- void Connect();
- void Subscribe(void \*topic);
- void Publisher(void * topic, void* message);

Arhitektura Servisa
![image](https://user-images.githubusercontent.com/17052851/206030929-9de3bf23-7e59-42ba-b309-4992e1910d77.png)
