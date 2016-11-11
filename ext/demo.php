<?php

define("CCARD_HASH_MURMUR", 1);
define("CCARD_HASH_LOOKUP3", 2);
define("CCARD_HASH_MURMUR64", 3);

echo "Initialize adaptive counting context 1 (k=13 or len=2^13) ...\n";
$ctx1 = adp_cnt_init(13, CCARD_HASH_MURMUR64);
assert(!is_null($ctx1));

$buf = adp_cnt_get_bytes($ctx1);
echo "Raw context buffer length: ", strlen($buf), "\n";

echo "Count distinct items ...\n";
for($i = 1; $i < 200000; $i++) {
    $is_new = adp_cnt_offer($ctx1, "$i");
    assert($is_new >= 0);
}
echo "Adaptive counting estimated: ", adp_cnt_card($ctx1), "\n";
echo "Loglog countinng estimated: ", adp_cnt_card_loglog($ctx1), "\n";

echo "Initialize adaptive counting context 2 (k=13 or len=2^13) ...\n";
$ctx2 = adp_cnt_init(13, CCARD_HASH_MURMUR64);
assert(!is_null($ctx2));

echo "Count distinct items ...\n";
for($i = 1; $i < 300; $i++) {
    $is_new = adp_cnt_offer($ctx2, "$i");
    assert($is_new >= 0);
}
echo "Adaptive counting estimated: ", adp_cnt_card($ctx2), "\n";
echo "Loglog countinng estimated: ", adp_cnt_card_loglog($ctx2), "\n";

echo "Reset counting context 1 ...\n";
$rc = adp_cnt_reset($ctx1);
assert($rc == 0);

echo "Recount distinct items ...\n";
for($i = 100; $i < 1000; $i++) {
    $is_new = adp_cnt_offer($ctx1, "$i");
    assert($is_new >= 0);
}
echo "Adaptive counting estimated: ", adp_cnt_card($ctx1), "\n";
echo "Loglog countinng estimated: ", adp_cnt_card_loglog($ctx1), "\n";

echo "Merging two counting contexts ...\n";
$rc = adp_cnt_merge($ctx1, $ctx2);
assert($rc == 0);

echo "Adaptive counting estimated: ", adp_cnt_card($ctx1), "\n";
echo "Loglog countinng estimated: ", adp_cnt_card_loglog($ctx1), "\n";

echo "Release counting contexts ...\n";
$rc = adp_cnt_fini($ctx1);
assert($rc == 0);
$rc = adp_cnt_fini($ctx2);
assert($rc == 0);

?>

