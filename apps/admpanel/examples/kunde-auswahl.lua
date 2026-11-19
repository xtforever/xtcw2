-- Ported from experimental/AdmPnl/kunde-auswahl.json
return {
    title = 'Kundenauswahl',
    script = 'auswahl.sh $kk',
    sql = { group = 'adminpanel' },
    fields = {
        {
            type = 'kuerzel',
            title = 'Kuerzel',
            variable = 'kk',
            extra_vars = {
                sql_query_str = 'select custid,dbservid from cust',
                sql_disp_field = '$custid ($dbservid)',
                sql_key_field = '$custid',
            },
        },
    },
}
