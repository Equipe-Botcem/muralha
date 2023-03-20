var sheet_id = "1ORWDG7p8SR7Gc1Rv1aV7i9vmgsD1V0QgQvaCtg4bCzE";
var sheet_name = "log";

// Função para remover espaços em branco de uma string
function removerEspacos(texto) {
  return texto.replace(/ /g, '');
}

// Função para obter o nome associado ao ID do cartão
function getNomeById(idcard) {
  var sheet = SpreadsheetApp.openById(sheet_id).getSheetByName('permicoes');
  var idcards = sheet.getRange("B1:B").getValues();
  var nomes = sheet.getRange("A1:A").getValues();
  var idcardSemEspacos = removerEspacos(idcard);

  for (var i = 0; i < idcards.length; i++) {
    var idcardsSemEspacos = removerEspacos(idcards[i][0]);
    if (idcardsSemEspacos === idcardSemEspacos) {
      return nomes[i][0];
    }
  }
  return 'desconhecido';
}

// Função para processar solicitações GET
function doGet(e) {
  var ss = SpreadsheetApp.openById(sheet_id);
  var sheet = ss.getSheetByName(sheet_name);

  var idcard = String(e.parameter.idcard);
  var permission = String(e.parameter.permission);

  // Insere uma nova linha na segunda posição (após os títulos das colunas)
  sheet.insertRowAfter(1);

  // Adiciona os valores de 'idcard' e 'permission' na nova linha
  sheet.getRange(2, 1).setValue(idcard);
  sheet.getRange(2, 2).setValue(permission);

  // Obtém a data e hora atual
  var now = new Date();

  // Adiciona a data e hora na coluna C
  sheet.getRange(2, 3).setValue(now);

  // Obtém o nome correspondente ao ID do cartão
  var nome = getNomeById(idcard);

  // Adiciona o nome na coluna D
  sheet.getRange(2, 4).setValue(nome);

  // Atualiza a lista de IDs permitidos
  concat_Idcard_permitidos();

  // Retorna a lista de IDs permitidos como resultado da função doGet
  return ContentService.createTextOutput(idlist);
}

// Função para criar uma lista de IDs permitidos
function concat_Idcard_permitidos() {
  var sheet = SpreadsheetApp.openById(sheet_id).getSheetByName('permicoes');
  var Avals = sheet.getRange("B1:B").getValues();
  var Alast = Avals.filter(String).length;
  var permitidos = sheet.getRange("C1:C").getValues();

  idlist = ""; // Reinicia a lista de IDs permitidos

  for (var i = 0; i < Alast; i++) {
    if (permitidos[i][0].toLowerCase() == 'sim') {
      idlist += Avals[i][0] + ",";
    }
  }
}
