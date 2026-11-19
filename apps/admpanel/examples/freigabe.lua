-- Ported from experimental/AdmPnl/freigabe.json
-- The legacy kuerzel field had no explicit SQL block, so use a shared default.
return {
    title = 'freigabe erstellen',
    script = './freigabe.sh -m $kk $kk$sandbox $srv $domain',
    sql = {
        group = 'adminpanel',
        default = {
            query = 'select custid,dbservid from cust',
            display = '$custid ($dbservid)',
            key = '$custid',
        },
    },
    fields = {
        { type = 'kuerzel', title = 'Kuerzel', variable = 'kk' },
        { type = 'option', title = 'sandbox', variable = 'sandbox',
          values = { 1, 2, 3, 4, 5, 6, 7 } },
        { type = 'option', title = 'server', variable = 'srv',
          values = { 'sandbox7', 'sandbox9', 'sandbox10' } },
        { type = 'input', title = 'Domain', variable = 'domain' },
    },
}
