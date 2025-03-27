# Client-Server-Sockets-TCP
Implementação um par de programas que operam no modelo cliente-servidor com comunicação requisição-resposta utilizando o protocolo TCP na linguagem C.

##

<h3>Introdução</h3>

O experimento requer a implementação do modelo cliente-servidor com comunicação requisição-resposta
utilizando o protocolo TCP, devido à necessidade de uma comunicação confiável e ordenada. A interação entre
os programas cliente e servidor inicia-se com a troca de confirmações, seguida pelo envio da mensagem
'ready' pelo cliente, indicando sua prontidão para o envio de informações, e a posterior confirmação do
servidor, sinalizando que está preparado para receber as mensagens.

##

<h3>Metodologia</h3>

Os programas foram implementados utilizando a linguagem de programação C em uma máquina virtual
Oracle VM VirtualBox com sistema operacional Ubuntu (64-bit) com a internet configurada no modo bridge
para que atribua um endereço IP único para o sistema da VM. Na execução do programa, é possível criar uma
conexão utilizando endereço IPv4 ou IPv6, para isso, foi configurado para que o servidor escute os dois tipos
de endereços para concluir a conexão com o cliente.
Para a análise de desempenho, foi criado um laço com seis repetições no programa do cliente para cada
tamanho de mensagem. Esse laço foi necessário para utilizar a mesma conexão criada desde a inicialização do
programa. Para cada execução do cliente, foi calculado o tempo (utilizando o gettimeofday) percorrido em
segundos entre o primeiro envio da mensagem “ready” até a chegada da última mensagem de confirmação
“bye” do servidor. Entre o período de início e fim, é enviado os nomes dos diretórios do cliente para o servidor.
Além disso, também foi calculado o throughput de cada execução.

</br>

##

<h3>Resultados</h3>

![image](https://github.com/user-attachments/assets/626519ae-cd2c-4d00-a71b-217139e44382)

![image](https://github.com/user-attachments/assets/2cef9382-4157-4847-96fc-186dbbafc92e) 

##

<h3>Análise</h3>

O throughput médio cresceu proporcionalmente com o tamanho da mensagem com um valor médio de tempo
bem próximo, isso mostra que o sistema está sendo subutilizado pois não chegou ao seu desempenho máximo
com esses experimentos. Em relação ao tempo médio, os experimentos com as menores mensagens (32 - 64
bytes) obtiveram a maior variação do tempo, essa variação está atrelada à comparação do custo fixo que toda
mensagem tem ao ser enviada. Porém, em mensagens menores, esse custo fixo é relativamente grande com o
tamanho da mensagem (para 32 bytes). A partir de 64 bytes, esse custo fixo é diluído em mais bytes e a
relação custo fixo/tamanho da mensagem diminui. Com isso, a eficiência do sistema aumenta ao decorrer dos
experimentos em que o tamanho da mensagem aumenta.
</br>
</br>
Destaques:
</br>
</br>
● Mensagem 32 bytes: Experimento com o menor número de bytes, teve o maior tempo médio e a
maior variância de tempo (indicando instabilidade).
</br>
● Mensagem 4096 bytes: Experimento com o maior número de bytes, teve a segunda maior variância
de tempo (indicando instabilidade).
