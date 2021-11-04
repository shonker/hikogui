// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "iso_15924.hpp"
#include <array>

struct iso_15924_info {
    std::string code;
    uint16_t nr;
};

constexpr std::array<iso_15924_info> iso_15924_table_by_code = {
    {"Adlm", 166}, {"Afak", 439}, {"Aghb", 239}, {"Ahom", 338}, {"Arab", 160}, {"Aran", 161}, {"Armi", 124}, {"Armn", 230},
    {"Avst", 134}, {"Bali", 360}, {"Bamu", 435}, {"Bass", 259}, {"Batk", 365}, {"Beng", 325}, {"Bhks", 334}, {"Blis", 550},
    {"Bopo", 285}, {"Brah", 300}, {"Brai", 570}, {"Bugi", 367}, {"Buhd", 372}, {"Cakm", 349}, {"Cans", 440}, {"Cari", 201},
    {"Cham", 358}, {"Cher", 445}, {"Chrs", 109}, {"Cirt", 291}, {"Copt", 204}, {"Cpmn", 402}, {"Cprt", 403}, {"Cyrl", 220},
    {"Cyrs", 221}, {"Deva", 315}, {"Diak", 342}, {"Dogr", 328}, {"Dsrt", 250}, {"Dupl", 755}, {"Egyd", 70},  {"Egyh", 60},
    {"Egyp", 50},  {"Elba", 226}, {"Elym", 128}, {"Ethi", 430}, {"Geok", 241}, {"Geor", 240}, {"Glag", 225}, {"Gong", 312},
    {"Gonm", 313}, {"Goth", 206}, {"Gran", 343}, {"Grek", 200}, {"Gujr", 320}, {"Guru", 310}, {"Hanb", 503}, {"Hang", 286},
    {"Hani", 500}, {"Hano", 371}, {"Hans", 501}, {"Hant", 502}, {"Hatr", 127}, {"Hebr", 125}, {"Hira", 410}, {"Hluw", 80},
    {"Hmng", 450}, {"Hmnp", 451}, {"Hrkt", 412}, {"Hung", 176}, {"Inds", 610}, {"Ital", 210}, {"Jamo", 284}, {"Java", 361},
    {"Jpan", 413}, {"Jurc", 510}, {"Kali", 357}, {"Kana", 411}, {"Khar", 305}, {"Khmr", 355}, {"Khoj", 322}, {"Kitl", 505},
    {"Kits", 288}, {"Knda", 345}, {"Kore", 287}, {"Kpel", 436}, {"Kthi", 317}, {"Lana", 351}, {"Laoo", 356}, {"Latf", 217},
    {"Latg", 216}, {"Latn", 215}, {"Leke", 364}, {"Lepc", 335}, {"Limb", 336}, {"Lina", 400}, {"Linb", 401}, {"Lisu", 399},
    {"Loma", 437}, {"Lyci", 202}, {"Lydi", 116}, {"Mahj", 314}, {"Maka", 366}, {"Mand", 140}, {"Mani", 139}, {"Marc", 332},
    {"Maya", 90},  {"Medf", 265}, {"Mend", 438}, {"Merc", 101}, {"Mero", 100}, {"Mlym", 347}, {"Modi", 324}, {"Mong", 145},
    {"Moon", 218}, {"Mroo", 264}, {"Mtei", 337}, {"Mult", 323}, {"Mymr", 350}, {"Nand", 311}, {"Narb", 106}, {"Nbat", 159},
    {"Newa", 333}, {"Nkdb", 85},  {"Nkgb", 420}, {"Nkoo", 165}, {"Nshu", 499}, {"Ogam", 212}, {"Olck", 261}, {"Orkh", 175},
    {"Orya", 327}, {"Osge", 219}, {"Osma", 260}, {"Ougr", 143}, {"Palm", 126}, {"Pauc", 263}, {"Pcun", 15},  {"Pelm", 16},
    {"Perm", 227}, {"Phag", 331}, {"Phli", 131}, {"Phlp", 132}, {"Phlv", 133}, {"Phnx", 115}, {"Plrd", 282}, {"Piqd", 293},
    {"Prti", 130}, {"Psin", 103}, {"Qaaa", 900}, {"Qabx", 949}, {"Ranj", 303}, {"Rjng", 363}, {"Rohg", 167}, {"Roro", 620},
    {"Runr", 211}, {"Samr", 123}, {"Sara", 292}, {"Sarb", 105}, {"Saur", 344}, {"Sgnw", 95},  {"Shaw", 281}, {"Shrd", 319},
    {"Shui", 530}, {"Sidd", 302}, {"Sind", 318}, {"Sinh", 348}, {"Sogd", 141}, {"Sogo", 142}, {"Sora", 398}, {"Soyo", 329},
    {"Sund", 362}, {"Sylo", 316}, {"Syrc", 135}, {"Syre", 138}, {"Syrj", 137}, {"Syrn", 136}, {"Tagb", 373}, {"Takr", 321},
    {"Tale", 353}, {"Talu", 354}, {"Taml", 346}, {"Tang", 520}, {"Tavt", 359}, {"Telu", 340}, {"Teng", 290}, {"Tfng", 120},
    {"Tglg", 370}, {"Thaa", 170}, {"Thai", 352}, {"Tibt", 330}, {"Tirh", 326}, {"Tnsa", 275}, {"Toto", 294}, {"Ugar", 40},
    {"Vaii", 470}, {"Visp", 280}, {"Vith", 228}, {"Wara", 262}, {"Wcho", 283}, {"Wole", 480}, {"Xpeo", 30},  {"Xsux", 20},
    {"Yezi", 192}, {"Yiii", 460}, {"Zanb", 339}, {"Zinh", 994}, {"Zmth", 995}, {"Zsye", 993}, {"Zsym", 996}, {"Zxxx", 997},
    {"Zyyy", 998}, {"Zzzz", 999}};

constexpr auto iso_15924_table_by_nr_init() noexcept
{
    auto r = std::array<iso_15924_info, 1000>{};

    for (ttlet &info : iso_15924_table_by_code) {
        r[info.nr] = info;
    }

    return r;
}

constexpr auto iso_15924_table_by_nr = iso_15924_table_by_nr_init();
