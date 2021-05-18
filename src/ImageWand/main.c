#include <stdio.h>
#include "merge.h"

int main()
{
    char *hexValue = "89504E470D0A1A0A0000000D494844520000010000000100010300000066BC3A250000000467414D410000B18F0BFC6105000000206348524D00007A26000080840000FA00000080E8000075300000EA6000003A98000017709CBA513C00000006504C5445800080FFFFFF56303D3E00000001624B474401FF022DDE0000000774494D4507E505111511077D4848330000001F4944415468DEEDC1010D000000C2A0F74F6D0E37A00000000000000000BE0D210000017F199CA70000002574455874646174653A63726561746500323032312D30352D31375432313A31373A30372B30303A3030856C5BA10000002574455874646174653A6D6F6469667900323032312D30352D31375432313A31373A30372B30303A3030F431E31D0000000049454E44AE426082";
    char *hexValue2 = "89504E470D0A1A0A0000000D49484452000001000000010008060000005C72A8660000091B49444154785EEDD603D330681947F1E7DA364D9A387172936B3226D7D66463B2CDB56D65DBB6F93EF69B6DDBE6F641CEEFFE06E75CFF3973CF82C700035903932507CE00030B0260040C840D0840F8F8D01910001B60206C4000C2C787CE8000D80003610302103E3E740604C00618081B1080F0F1A133200036C040D88000848F0F9D0101B00106C20604207C7CE80C08800D3010362000E1E3436740006C8081B00101081F1F3A030260030C840D0840F8F8D01910001B60206C4000C2C787CE8000D80003610302103E3E740604C00618081B1080F0F1A133200036C040D88000848F0F9D0101B00106C20604207C7CE80C08800D3010362000E1E3436740006C8081B00101081F1F3A030260030C840D0840F8F8D01910001B60206C4000C2C787CE8000D80003610302103E3E740604C00618081B1080F0F1A133200036C040D88000848F0F9D0101B00106C20604207C7CE80C08800D3010362000E1E3436740006C8081B00101081F1F3A030260030C840D0840F8F8D01910001B60206C4000C2C787CE8000D80003610302103E3E740604C00618081B1080F0F1A133200036C040D88000848F0F9D0101B00106C20604207C7CE80C08800D3010362000E1E3436740006C8081B00101081F1F3A030260030C840D0840F8F8D01910001B60206C4000C2C787CE8000D80003610302103E3E740604C00618081B1080F0F1A133200036C040D88000848F0F9D0101B00106C20604207C7CE80C08800D3010362000E1E3436740006C8081B00101081F1F3A030260030C840D0840F8F8D01910001B60206C4000C2C787CE8000D80003610302103E3E740604C00618081B1080F0F1A133200036C040D88000848F0F9D0101B00106C20604207C7CE80C08800D3010362000E1E3436740006C8081B00101081F1F3A030260030C840D0840F8F8D01910001B60206C4000C2C787CE8000D80003610302103E3E740604C00618081B1080F0F1A133200036C040D88000848F0F9D0101B00106C20604207C7CE80C08800D3010362000E1E3436740006C8081B081397BEFF3CE09F34367206D60CEDABBBF00A42700BE6C60CEDC3D4000CA0BC09E363067EC1E2800E909802F1B98D3770F1280F202B0A70DCC69BB070B407A02E0CB06E6D4DD4304A0BC00EC690373CAEEA102909E00F8B2813979F73001282F007BDAC09CB47BB800A42700BE6C604EDC3D4200CA0BC09E363027EC1E2900E909802F1B98E3778F1280F202B0A70DCC713B470B407A02E0CB06E6D89D6304A0BC00EC690373CCCEB102909E00F8B281397AE73801282F007BDAC01CB573BC00A42700BE6C608EDC394100CA0BC09E363047EC9C2800E909802F1B98C3774E1280F202B0A70DCC613B270B407A02E0CB06E6D09D5304A0BC00EC690373C8F6A902909E00F8B2813978FB3401282F007BDAC01CB47DBA00A42700BE6C600EDC3E4300CA0BC09E3630076C9F2900E909802F1B98FDB7CF1280F202B0A70DCC73B7CF1680F404C0970DCC73B69F2F00E505604F1B98676FBF4000D213005F3630CFDA7EA1009417803D6D609EB9FD2201484F007CD9C03C63EBC502505E00F6B48179FAD64B04203D01F06503F3B4AD970A407901D8D306E6A95B2F1380F404C0970DCC53B65E2E00E505604F1B98276FBD4200D213005F36304FDA7AA5009417803D6D609EB8F52A01484F007CD9C03C61EBD502505E00F6B48179FCD66B04203D01F06503F3B8ADD70A407901D8D306E6B19BAF1380F404C0970DCC63365F2F00E505604F1B98476FBE4100D213005F36308FDA7CA3009417803D6D601EB9F92601484F007CD9C03C62F3CD02505E00F6B48179F8E65B04203D01F06503F3B0CDB70A407901D8D306E6A19B6F1380F404C0970DCC4336DF2E00E505604F1B98076FBC4300D213005F36300FDA78A7009417803D6D601EB8F12E01484F007CD9C03C60E3DD02505E00F6B481B9FFC67B04203D01F0650373BF8DF70A407901D8D306E6BE1BEF1380F404C0970DCC7D36DE2F00E505604F1B987B6F7C4000D213005F3630F7DAF8A0009417803D6D60EEB9F12101484F007CD9C0DC63FDC302505E00F6B481B9FBFA4704203D01F0650373B7F58F0A407901D8D306E6AEEB1F1380F404C0970DCC7EEB1F1780F202B0A70DCC5DD63F2100E909802F1B983BAF7F5200CA0BC09E3630775AFF9400A42700BE6C60EEB8FE6901282F007BDAC0DC61FD3302909E00F8B281B9FDDA6705A0BC00EC690373BBB5CF09407A02E0CB06E6B66B7B04A0BC00EC6903739BB54501484F007CD9C0DC7A6D4900CA0BC09E3630B75A5B1680F404C0970DCC2DD75604A0BC00EC6903738BB55501484F007CD9C0DC7C6D4D00CA0BC09E3630375B5B1780F404C0970DCC4DD73604A0BC00EC69037393D54D01484F007CD9C0DC78754B00CA0BC09E3630375ADD1680F404C0970DCC0D577704A0BC00EC69037383D55D01484F007CD9C05C7F75AF009417803D6D60AEB7FA7901484F007CD9C05C77F50B02505E00F6B481B9CEEA1705203D01F0650373EDD52F09407901D8D306E65A2B5F1680F404C0970DCC3557BE2200E505604F1B986BAC7C5500D213005F3630575FF99A009417803D6D60AEB6F27501484F007CD9C05C75E51B02505E00F6B481B9CACA3705203D01F0650373E5956F09407901D8D306E64A2BDF1680F404C0970DCC1557BE2300E505604F1B982BAC7C5700D213005F3630975FFE9E009417803D6D602EB7FC7D01484F007CD9C05C76F90702505E00F6B481B9CCF20F05203D01F0650373E9E51F09407901D8D306E652CB3F1680F404C0970DCC25977F2200E505604F1B984B2CFF5400D213005F3630175FFE99009417803D6D602EB6FC7301484F007CD9C05C74F91702505E00F6B481B9C8D22F05203D01F0650373E1A55F09407901D8D306E6424BBF1680F404C0970DCC05977E2300E505604F1B980B2CFD5600D213005F3630E75FFA9D009417803D6D60CE2700E901806F1B98F32EFDDE0FA0BD01F46103739EA53F08407800D0DB06E6DC4B7F1480F606D0870DCCBE8B7F1280F000A0B70DCCBE8B7F1680F606D0870DCCB916FF2200E101406F1B987D16FF2A00ED0DA00F1B987D16FF2600E101406F1B9859FCBB00B437803E6C6066F11F02101E00F4B68199C57F0A407B03E8C3066661F15F02101E00F4B6815958FCB700B437803E6C601616FF2300E101406F1B98853DFF1580F606D0870DCCC29EFF09407800D0DB066661CF3902D0DE00FAB081FF035B678C30319972E10000000049454E44AE426082";
    char *filename = "sss.png";
    unsigned char *blob = merge(hexValue, hexValue2);
    printf("%s\n", blob);
    return 0;
}