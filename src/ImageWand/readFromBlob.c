#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MagickWand/MagickWand.h>

#define ThrowWandException(wand)                                                          \
    {                                                                                     \
        char                                                                              \
            *description;                                                                 \
                                                                                          \
        ExceptionType                                                                     \
            severity;                                                                     \
                                                                                          \
        description = MagickGetException(wand, &severity);                                \
        (void)fprintf(stderr, "%s %s %lu, reason: %s\n", GetMagickModule(), description); \
        description = (char *)MagickRelinquishMemory(description);                        \
        exit(-1);                                                                         \
    }

int main(int argc, char **argv)
{

    MagickBooleanType status;
    MagickWand *magick_wand, *mw2;

    /*
    Read an image.
  */

    MagickWandGenesis();
    magick_wand = NewMagickWand();
    mw2 = NewMagickWand();

    status = MagickReadImage(magick_wand, "/home/roees/Desktop/img1.png");
    if (status == MagickFalse)
        ThrowWandException(magick_wand);

    MagickResetIterator(magick_wand);
    while (MagickNextImage(magick_wand) != MagickFalse)
    {
        // unsigned char *blob = argv[1];
        size_t length;
        unsigned char *blob = MagickGetImageBlob(magick_wand, &length);
        // printf("%s \n", blob);
        // printf("%ld \n", length);
        // unsigned char *blob = "89504E470D0A1A0AFFD8FFE000104A46494600010100000100010000FFDB004300080606070605080707070909080A0C140D0C0B0B0C1912130F141D1A1F1E1D1A1C1C20242E2720222C231C1C2837292C30313434341F27393D38323C2E333432FFDB0043010909090C0B0C180D0D1832211C213232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232FFC00011080100010003012200021101031101FFC4001A000101010101010100000000000000000000010203040507FFC4002E10000102060103030305000300000000000001120211135161A15203149221536231549104224163A2323342FFC400190101010101010100000000000000000000000102040305FFC4001D110100030101010101010000000000000000011112210213220351FFDA000C03010002110311003F00FC48199899DEF0681999450A080894A09324C0D0333132D2B40CCC4C0D033313250D033329685040129410014100500000A40285066626129A133332829410028001140014000440000000140000000000000000001400010050402140190505542800000112626504549940280008800000980152626500000004C0024C4CA000000000A000000020000A000000022000280002800000022201228500054000400000001400000000000000014001100015541011005216000000004000140020550000000400000000000001001410014001400040001400A100001014800164240412280A921228142481401048B22C823321235212033212352120332123521220CC848D48480CC8B22C8146642468480CC848D48480CC81A912404122C8B20ADB178AFE03178E8D5786CA2BC36537F967ACB178E833E3A355E1B28AF0D949F94EB2C5E3A0C5E2BF8355E1B28AF0D947E4EB2C5E2BF80C5E2BF8375E1B292BC3652F0EB2C5E3A0C5E3A355E1B28AF0D949C3ACB578E83178AFE0D5786CA5AD0D9470EB0C5E2BF82B178AFE0D5786CA2BC3651C5EB2C5E2BF80C5E2BF8355E1B28AF0D947E4EB2C5E2BF80C5E2BF8355E1B28AF0D9470EB2D5E3A23578E8D5786CA2BC3652F13ACB578E83578E8D5786CA2BC3651C3ACB578E83578E8D5786CA2BC3651C3ACB578E83578E8D5786CA2BC365170BD46AF1D06FC745AF0D945786CA38751ABC741ABC745AF0D94B5E1B28FCA7596AF1D06FC746ABC36515E1B28E1D629437514A1BA9E86FC741BF1D0C41A79E9C37514E1BA9E86FC741BF1D17109A79E9C37514E1BA9E86FC741BF1D0C41A79E9437514E1BA9DDB8D06FC74310B6E14E1BA8A70DD4F437E3A0DF8E866134F3D386EA54E9C3753BB7E3A0DC68620B70A50DD45286EA7A1B8D06FC74330BA79E9437514A1BA9E86FC741BF1D0CC1A79E9437514A1BA9E86FC741B8D0CC1A79A943751461BA9E96E341B8D0CC26A5E6A30DD45186EA7A5B8D06E343306A5E7A50DD45286EA7A1BF1D06FC7433069E7A50DD45286EA7A1BF1D06FC7431069E7A50DD45186EA7A1B8D15B8D0C41A979A8C37514A1BA9E96E341A9C745C41A79A9437514A1BA9E86E341B8D0C41A74906A87603B06D8590911D80EC01644907603B00497A964262650906A87603B0406A86A87603B00242447603B00A590911D80EC01642447603B00A56A86A848BD3E81D8012120E23B00590911D80EC029A90911D80EC01644907608EC029642447603B02CA590911D82CC594CAFD0CCD4D2F4A3B6C94E3B6C936BC26A49A969476D8A51DB64EAF126A26A2947C7654E9C72FA6C74E32E5B85896E6A9C76D8A71DB63A5C32912872CFEA6A9C76D8A51DB62A4B865CB70E5B9AA71DB629C76D8ED9C666A49A9D29476D929476D93A5C30E5B872DCDD28EDB14A3E3B1D2E1872DC8E5B9D2947C762947C762A4B87348E2B95F15CDD28F8EC528F8EC54970E6F8AE1F15CE94A3B6C528F8EC54970E4F8AE1F15CEB4A3E3B14A3E3B1525C393E2B87C573AD28F8EC528F8EC54ADC393E2B87C573AD28F8EC528EDB152970E4F8AE1F15CEB4A3B6C528F8EC9992E1C57A91A2FD4558EFA3BA74E34FE364A71DB6332B70F6AF4FD3EA669E746E6A0E8A78B14F3A14F3A362628629E7429E746C4C516CD2CE852CE8D4D44D4516CD2CE852CE8D4D44D416CD2CE852CE8D4D44D417294BD3EBA252CE8D396E472938759A79D169E741CA1CB72F0E94F3A14F3A0E5B872DC97074A79D0A79D072DC396E2E0E94B3A14F3A0E5B91F15C5C1D5A79D0A79D11F15C3E2B8E1D6A9E7429E48F8AE47C571C3AB4F229E48F8AE47C571C3AD53C8A7932F8AE1F15C5C2F5B4E94FFF005A147E5A315224FE4CAF5A3F5FDDA17074452953A7EBF52D3C928E300DD3CE853CE85170C9154DD3CE88BD3F5FAE85170C48494E8CC8664516C48494DB32199145B0A8A24A74587246E4B5D2D892854536DC86E4945B9C94B2536DC86E4516C494494E8DC86E4516E7252B56C6A451496E6D5B06AD8E8490A5B736AD8356C749090A2D86AD8356C6E42428736AD8356C7491645A2DCDB158356C74916428B612159164A6A406612DA9203D1252494D5719B70077928928A2DC048EF25125145BCE591E86A86AD8B45BCF2123BAC2B60D5B0A2DC242487A1AA46A8A2DC248248776AD8AB0AD8516F349048F4356C1AB62516E12123BB56C1AB62D16E0D42490F4B56C1AB6145BCD241243D2D5B11AB6192DE79215A963BB56C56AD8516F3B52C4921E96AD8356C325BCD242C90F4B22B064561934F3A429634904363B322B15AB61496E0C86C190D8EEC8AC19158B45B4B0FA7D4CB72715EB472FF968CD68F9689A8332F4B721B93CF5A3E5A15A3E5A2EA0CCBBB7256E4F3568F968B5A3E5A1A8332F403CF523BE8548EFA1A832F4091E7A91DF45A91DF44D4199779091C2A477D0A91DF435054BBC848E3522B92A477D09929DE42470A91DF42A477D1624A779091C2AC77D0A91DF42E0A77071A915CAF8AE2CA750727C570F8AE2C750727C570F8AE2D1D6654FA9C922534E5053AA099CD225B872DCB694E80E6E5B872DC5C14E885393E24FE43E2B8B29F117F51E9FF77FA2771FDDFE8F12C0B2FE0CB54F9D3FD26DDB1E21EFEE3FBBFD0EE3FBBFD1E06A91AA27FA4AE21EFEE3FBBFD17B8FEEFF0047CF5854A90AC87D24C43DDDC2FBCBE44EE17DE5F23C4D523547D3D1887BBB85F797C8772BEF2F91E162E031703E9E8C43DDDCC5EF2F90EE62F797C8F0B17018B827D3D1987B7B98BDF5F3277317BCBE478D8B80C5C0FA7A310F677317BEBE65EE62F7D7CCF0AC0B80C5C0FA7A310F6F7317BCBE63B98BDF5F33C2C5C158B81F4F4B887AD7F531CFD3AF1798EE7A9EFC5E6791205C06A93E9E8CC3D7DCF53DF8BCC773D4F7E2F33C8D50D51F4F4661EBEEBA9EFC5E63BAEA7BF1799E36A9243EBE8CC3D517EABACE5975FA92C46A67BAEBFDC753CD4E1F4248CEFD2E61E8EEFAFF0071D4F351DDF5FEE3A9E6A79E4490DFA5CC3D3DDF5FEE3A9E6A3BBEBFDC753CD4F3C8486FD198775FD5FEA3EE3ABE6A4EEBF51F71D5F353881BF4661BAB12FF00084A8B643524B21249641369C4A91590545C164964124B213AB68F5C07AE0B24B0925874B47AE0545C1B924BE884925907466A2E03D706A4964124B216A466A2E0545C1A94364124B212A4E3351702A2E0DCA1B20943642D4FFA718A8B8151706E50D904A1B20A94B737AE0AF5C1B943642CA1B20A92DC97A8B815170764482D0894168454970E351701EB83ACA0B42250D905496E7324CEB286C8250E0516E6246FD303D30325B9C848E9E981E98195B624491D6498124B20CA5B9C8350EBFB703F6E0656DFFFD9";

        status = MagickReadImageBlob(mw2, blob, length);
        if (status == MagickFalse)
            ThrowWandException(magick_wand);

        status = MagickWriteImages(mw2, "output.png", MagickTrue);
        if (status == MagickFalse)
            ThrowWandException(magick_wand);

        mw2 = DestroyMagickWand(mw2);
    }
    magick_wand = DestroyMagickWand(magick_wand);
    MagickWandTerminus();
    return (0);
}