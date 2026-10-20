<?php
//$userKey = $_POST["key"];

$config = array(
    //"digest_alg" => "sha512",
    "private_key_bits" => 4096,
    "private_key_type" => OPENSSL_KEYTYPE_RSA,
);

/*
function encryptData($value){
   $key = $userKey;
   $text = $value;
   $iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_ECB);
   $iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);
   $crypttext = mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $text, MCRYPT_MODE_ECB, $iv);
   return $crypttext;
}
*/
// Create the keypair
$res = openssl_pkey_new($config);

// Get private key
openssl_pkey_export($res, $privKey);

// Get public key
$keyDetails = openssl_pkey_get_details($res);
if (isset($keyDetails['rsa']))
{
	$pubKey = $keyDetails["key"];
    $exponentBinary = strrev($keyDetails['rsa']['e']);
	$modulusBinary = strrev($keyDetails['rsa']['n']); 
    $prime1Binary = strrev($keyDetails['rsa']['p']);
	$prime2Binary = strrev($keyDetails['rsa']['q']);
    $exponent1Binary = strrev($keyDetails['rsa']['dmp1']);
    $exponent2Binary = strrev($keyDetails['rsa']['dmq1']);
    $coefficientBinary = strrev($keyDetails['rsa']['iqmp']);
    $privateExponentBinary = strrev($keyDetails['rsa']['d']);
	$bufPubKey = "\x06\x02\x00\x00\x00\xa4\x00\x00" . "RSA1" . pack("L", $keyDetails['bits']) . $exponentBinary . "\0" . $modulusBinary;
	$bufPrivKey = "\x07\x02\x00\x00\x00\xa4\x00\x00" . "RSA2" . pack("L", $keyDetails['bits']) . $exponentBinary . "\0" . $modulusBinary . $prime1Binary . $prime2Binary . $exponent1Binary . $exponent2Binary . $coefficientBinary . $privateExponentBinary;
    file_put_contents('./PubKey.bin', $bufPubKey);
	file_put_contents('./PrivKey.bin', $bufPrivKey);
	header('Content-Type: application/octet-stream');
	header('Content-Length: ' . strlen($bufPubKey));
	header('Content-Disposition: attachment; filename="PubKey.bin"');
	header('Content-Transfer-Encoding: binary');
	echo $bufPubKey;
}
?>