/***************************************************************************
 *   Copyright (C) 2006 by ASTRON, Adriaan Renting                         *
 *   renting@astron.nl                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <tables/Tables.h>
#include <tables/Tables/TableIter.h>
#include "BandpassCorrector.h"
#include <casa/Quanta/MVEpoch.h>

// Oo nice, hardcoded values. These are the bandpass shape. Source: andre Gunst
double StaticBandpass[] = {
	{ //Bandpass based on scaled Andre Gunst data -0.5 *2
	0.479170562066 , 0.514502630718 , 0.550139571562 , 0.585848684286
	0.621388651222 , 0.65651292434 , 0.690973343801 , 0.724523922439
	0.756924724094 , 0.787945758817 , 0.817370814683 , 0.84500114455
	0.870658926603 , 0.894190420023 , 0.915468741612 , 0.934396195637
	0.950906097416 , 0.96496404113 , 0.976568573774 , 0.985751249814
	0.992576054715 , 0.997138199701 , 0.999562304602 , 1.000000000000
	0.998626993792 , 0.995639660366 , 0.991251222454 , 0.985687606084
	0.979183057608 , 0.971975618259 , 0.964302555881 , 0.956395855297
	0.948477868061 , 0.940757219114 , 0.933425062193 , 0.926651767797
	0.920584117312 , 0.915343064773 , 0.911022113982 , 0.907686343678
	0.905372097502 , 0.90408733911 , 0.903812656309 , 0.904502881999
	0.906089284456 , 0.908482265414 , 0.911574492001 , 0.915244378041
	0.919359822016 , 0.923782103132 , 0.928369833781 , 0.932982866204
	0.93748605341 , 0.941752769337 , 0.945668100668 , 0.949131632495
	0.952059761845 , 0.954387486601 , 0.956069632238 , 0.957081494594
	0.957418893157 , 0.957097645643 , 0.956152490516 , 0.954635499025
	0.952614032057 , 0.950168309023 , 0.947388666004 , 0.944372588018
	0.941221605461 , 0.938038147277 , 0.934922443305 , 0.931969565373
	0.92926669135 , 0.926890668567 , 0.924905943098 , 0.923362909569
	0.922296722964 , 0.921726599499 , 0.921655618686 , 0.922071023505
	0.922945000688 , 0.924235908886 , 0.925889909405 , 0.927842942597
	0.930022983253 , 0.932352500725 , 0.934751044251 , 0.937137871132
	0.939434535208 , 0.941567355363 , 0.943469688526 , 0.945083938694
	0.946363242554 , 0.947272783148 , 0.947790695243 , 0.947908539346
	0.947631335159 , 0.946977159288 , 0.945976325769 , 0.944670181013
	0.943109556708 , 0.941352934682 , 0.939464386378 , 0.937511356218
	0.935562362488 , 0.93368469135 , 0.931942159144 , 0.930393015312
	0.929088053118 , 0.928068988108 , 0.927367155106 , 0.927002563827
	0.926983341267 , 0.927305576236 , 0.927953568206 , 0.928900469411
	0.930109296391 , 0.93153427515 , 0.933122473411 , 0.934815664194
	0.936552357637 , 0.938269932678 , 0.939906797177 , 0.941404504361
	0.942709755128 , 0.943776219671 , 0.944566117991 , 0.945051506954
	0.945215231269 , 0.945051506954 , 0.944566117991 , 0.943776219671
	0.942709755128 , 0.941404504361 , 0.939906797177 , 0.938269932678
	0.936552357637 , 0.934815664194 , 0.933122473411 , 0.93153427515
	0.930109296391 , 0.928900469411 , 0.927953568206 , 0.927305576236
	0.926983341267 , 0.927002563827 , 0.927367155106 , 0.928068988108
	0.929088053118 , 0.930393015312 , 0.931942159144 , 0.93368469135
	0.935562362488 , 0.937511356218 , 0.939464386378 , 0.941352934682
	0.943109556708 , 0.944670181013 , 0.945976325769 , 0.946977159288
	0.947631335159 , 0.947908539346 , 0.947790695243 , 0.947272783148
	0.946363242554 , 0.945083938694 , 0.943469688526 , 0.941567355363
	0.939434535208 , 0.937137871132 , 0.934751044251 , 0.932352500725
	0.930022983253 , 0.927842942597 , 0.925889909405 , 0.924235908886
	0.922945000688 , 0.922071023505 , 0.921655618686 , 0.921726599499
	0.922296722964 , 0.923362909569 , 0.924905943098 , 0.926890668567
	0.92926669135 , 0.931969565373 , 0.934922443305 , 0.938038147277
	0.941221605461 , 0.944372588018 , 0.947388666004 , 0.950168309023
	0.952614032057 , 0.954635499025 , 0.956152490516 , 0.957097645643
	0.957418893157 , 0.957081494594 , 0.956069632238 , 0.954387486601
	0.952059761845 , 0.949131632495 , 0.945668100668 , 0.941752769337
	0.93748605341 , 0.932982866204 , 0.928369833781 , 0.923782103132
	0.919359822016 , 0.915244378041 , 0.911574492001 , 0.908482265414
	0.906089284456 , 0.904502881999 , 0.903812656309 , 0.90408733911
	0.905372097502 , 0.907686343678 , 0.911022113982 , 0.915343064773
	0.920584117312 , 0.926651767797 , 0.933425062193 , 0.940757219114
	0.948477868061 , 0.956395855297 , 0.964302555881 , 0.971975618259
	0.979183057608 , 0.985687606084 , 0.991251222454 , 0.995639660366
	0.998626993792 , 1.000000000000 , 0.999562304602 , 0.997138199701
	0.992576054715 , 0.985751249814 , 0.976568573774 , 0.96496404113
	0.950906097416 , 0.934396195637 , 0.915468741612 , 0.894190420023
	0.870658926603 , 0.84500114455 , 0.817370814683 , 0.787945758817
	0.756924724094 , 0.724523922439 , 0.690973343801 , 0.65651292434
	0.621388651222 , 0.585848684286 , 0.550139571562 , 0.514502630718
	},
	{ //BANDPASS_150MHZ as used by Stefan de Koning
	0.464000000000 , 0.47040562257 , 0.478945454065 , 0.492786452134
	0.512184482713 , 0.536971668916 , 0.564654921566 , 0.59532504034
	0.629704190144 , 0.665699613786 , 0.702710829829 , 0.74039778589
	0.777518252518 , 0.813720273522 , 0.847547218473 , 0.878659034997
	0.906759318042 , 0.930910509747 , 0.951748869137 , 0.96846083644
	0.981046279397 , 0.989630452608 , 0.994320754965 , 0.994579001667
	0.992062137925 , 0.986096407693 , 0.977545366769 , 0.966999444488
	0.954646853423 , 0.94051794831 , 0.926395259636 , 0.911597558393
	0.896483414014 , 0.882395643203 , 0.869189482315 , 0.856819696849
	0.846352604291 , 0.837057971588 , 0.82932485517 , 0.823375062825
	0.819050141524 , 0.816768907228 , 0.816497764726 , 0.817878210724
	0.82059552152 , 0.824655450625 , 0.830307052351 , 0.836859111711
	0.844171031398 , 0.85203720604 , 0.860760124857 , 0.8685698225
	0.876879083672 , 0.885204150463 , 0.892074372405 , 0.898430547839
	0.90410139407 , 0.908509972752 , 0.911848927333 , 0.91386457424
	0.914602147978 , 0.915033925879 , 0.912458468903 , 0.909365262017
	0.905859191068 , 0.900966788351 , 0.895847684045 , 0.890456775916
	0.884686466681 , 0.878907494115 , 0.873054321085 , 0.867685302752
	0.862838994261 , 0.858746594187 , 0.854794262361 , 0.852819485226
	0.850364984261 , 0.849486085759 , 0.84974294368 , 0.849728328439
	0.851520051318 , 0.854295227891 , 0.857460651271 , 0.860468547468
	0.864789302437 , 0.869033145517 , 0.873737137265 , 0.877950757877
	0.882110282253 , 0.88615625744 , 0.889281736899 , 0.8928840965
	0.895031611247 , 0.896725987355 , 0.897305769382 , 0.897886080469
	0.897298230299 , 0.89615288469 , 0.894353490463 , 0.891944488531
	0.889635081872 , 0.886586911091 , 0.882740721637 , 0.878936922994
	0.875557958364 , 0.871707338042 , 0.868823439938 , 0.866042575987
	0.8634982012 , 0.861516678571 , 0.860517882177 , 0.859942266486
	0.859522127872 , 0.860178028199 , 0.861423564267 , 0.863247162924
	0.865407507339 , 0.868030976378 , 0.871264516043 , 0.874183662672
	0.877442464884 , 0.880580575615 , 0.883843081235 , 0.886529309843
	0.889131352008 , 0.891041121074 , 0.892240629051 , 0.893446684124
	0.893467780335 , 0.893301126895 , 0.892367933763 , 0.890813625903
	0.888706716397 , 0.886601460201 , 0.884054307859 , 0.881044626088
	0.877636298708 , 0.875130412931 , 0.871850712908 , 0.870265587388
	0.866918366267 , 0.864149207735 , 0.862544242521 , 0.861397442003
	0.860665952439 , 0.860750866332 , 0.861175039018 , 0.862534388805
	0.864651284288 , 0.866788482397 , 0.869454408379 , 0.87303758961
	0.8761565881 , 0.879914292517 , 0.883592572019 , 0.886772544508
	0.890454527417 , 0.893050353147 , 0.8956965717 , 0.897627899902
	0.898658439806 , 0.899128971246 , 0.899127780864 , 0.898099026533
	0.896176361664 , 0.894282464355 , 0.891674933206 , 0.887932703754
	0.883779263549 , 0.879585945559 , 0.875149789698 , 0.870469407189
	0.866955069704 , 0.862639340792 , 0.859009205618 , 0.856268550116
	0.853854323731 , 0.852722138454 , 0.851672221781 , 0.851799394228
	0.853069663782 , 0.855086633335 , 0.858176136813 , 0.861489498187
	0.865652461445 , 0.870606631749 , 0.875771433482 , 0.881544520275
	0.887392931776 , 0.892948178715 , 0.898584437743 , 0.90443073301
	0.909121366028 , 0.912664471074 , 0.915097479036 , 0.916754887178
	0.917649260639 , 0.916668319975 , 0.91531498823 , 0.911979406396
	0.908169325186 , 0.90193847049 , 0.896158572071 , 0.888470756287
	0.880730497579 , 0.872509258523 , 0.863739980952 , 0.8557530487
	0.847748525248 , 0.840041861758 , 0.833505211228 , 0.828005383171
	0.823698053063 , 0.821135888687 , 0.819645200117 , 0.820532034494
	0.822753815833 , 0.82704117398 , 0.83301166574 , 0.840992910615
	0.849990675344 , 0.861220339655 , 0.873229836257 , 0.88703006375
	0.901300425891 , 0.916323043146 , 0.931011427665 , 0.945597174828
	0.959472991562 , 0.971859508504 , 0.982618310717 , 0.991236542074
	0.996971999577 , 1.000000000000 , 0.999291656745 , 0.994720326428
	0.986476139458 , 0.973909478082 , 0.957031650928 , 0.936090125123
	0.911487911013 , 0.883416197127 , 0.851988995581 , 0.817449937835
	0.781778231887 , 0.744177644632 , 0.706185554057 , 0.668572666191
	0.632288508848 , 0.597643771659 , 0.566066582016 , 0.537928768881
	0.513338855382 , 0.494006031267 , 0.479737586964 , 0.470877807316
	},
	{ //bandpass based on 20 augustus HBA data, band 31
	0.457000000000 , 0.463127454743 , 0.472220711418 , 0.486950338012
	0.506848752083 , 0.531513162438 , 0.560359234761 , 0.592669860076
	0.627768854647 , 0.664730275035 , 0.702804594295 , 0.741285665919
	0.779117129089 , 0.815658676657 , 0.850258217699 , 0.88217581432
	0.910810641843 , 0.935971774658 , 0.956945368839 , 0.973932695873
	0.986498967923 , 0.995535514678 , 1.000000000000 , 0.999993905188
	0.997059643859 , 0.991165422644 , 0.982422597764 , 0.97142983824
	0.958451026415 , 0.944543772405 , 0.929655360387 , 0.914515833088
	0.899459612419 , 0.884905223448 , 0.871107594306 , 0.858650094789
	0.847396299578 , 0.837895079602 , 0.829857700293 , 0.823893808262
	0.819682742812 , 0.817359509942 , 0.816720177511 , 0.817875626177
	0.8207514184 , 0.824918275727 , 0.830472404969 , 0.837231995314
	0.844564590011 , 0.852582779723 , 0.861058389895 , 0.869505038495
	0.87794669552 , 0.886108399136 , 0.893191489595 , 0.899481791847
	0.904893207 , 0.909085994355 , 0.91243436376 , 0.914363485086
	0.914937957103 , 0.914127802489 , 0.912296668094 , 0.909526240166
	0.90564389347 , 0.900968745611 , 0.896051123982 , 0.89186753386
	0.885153773096 , 0.880402011478 , 0.873377516345 , 0.867483939843
	0.862365740144 , 0.858024579356 , 0.85443652492 , 0.851490584393
	0.849383175529 , 0.848681720127 , 0.848406081609 , 0.84897581832
	0.850498090852 , 0.852862391005 , 0.855938081849 , 0.859284004436
	0.863238603612 , 0.867595032399 , 0.872046622367 , 0.876494058726
	0.880671554271 , 0.884562386311 , 0.888151232979 , 0.891035209944
	0.893432141051 , 0.895073071041 , 0.895792819962 , 0.896069546229
	0.895447155115 , 0.894383618341 , 0.892281121688 , 0.889829649958
	0.886903862153 , 0.883729918662 , 0.880015667684 , 0.876170173431
	0.87263934277 , 0.86920108779 , 0.865729229954 , 0.863400745153
	0.860683435969 , 0.858681783442 , 0.857313052597 , 0.856415640102
	0.856313579432 , 0.857017784949 , 0.85813440392 , 0.859746011133
	0.861939829552 , 0.864496655935 , 0.867439753939 , 0.870776679146
	0.873740417262 , 0.876831300112 , 0.879750012055 , 0.882883114007
	0.885431374068 , 0.887060614471 , 0.888225697855 , 0.889008684951
	0.8894 , 0.888950137103 , 0.888348852917 , 0.886522863924
	0.884668892828 , 0.882026984454 , 0.879196198212 , 0.875847596519
	0.874050452077 , 0.869766054062 , 0.866580978896 , 0.864089839199
	0.862308485737 , 0.858843753674 , 0.856962651154 , 0.855682504884
	0.8547982653 , 0.854847556053 , 0.855637022944 , 0.856988348742
	0.858810567151 , 0.862191792548 , 0.863871475005 , 0.867116159133
	0.871138512571 , 0.873 , 0.878002139332 , 0.881523134222
	0.884535074595 , 0.887342637977 , 0.889793462968 , 0.891649870105
	0.892763768492 , 0.8937 , 0.892834562225 , 0.891713033164
	0.890039567507 , 0.887777601049 , 0.884473750611 , 0.880794864408
	0.876852341263 , 0.872648796119 , 0.868212016208 , 0.863566572206
	0.859262793649 , 0.855171955992 , 0.851873897183 , 0.848536603102
	0.846133645802 , 0.84443485527 , 0.844803970476 , 0.843718094864
	0.844762286407 , 0.846681902903 , 0.849829821785 , 0.853336167028
	0.857527424824 , 0.86230932537 , 0.86767744088 , 0.873549854191
	0.879323978231 , 0.885192589526 , 0.890724368466 , 0.895880152856
	0.900513779142 , 0.904555493803 , 0.90697142645 , 0.908720490826
	0.909540434689 , 0.90877076035 , 0.906585057694 , 0.903418879439
	0.899043486666 , 0.893772837066 , 0.887070255371 , 0.879760616115
	0.871844618422 , 0.863500960804 , 0.85492750797 , 0.846354216128
	0.838500467406 , 0.831145656364 , 0.824371684243 , 0.818858641228
	0.814580047908 , 0.811636615044 , 0.810258576048 , 0.810793692434
	0.813065464243 , 0.817174942857 , 0.823111629062 , 0.830839976504
	0.840437405173 , 0.851247930275 , 0.863694583938 , 0.877319119517
	0.891929794667 , 0.906441824994 , 0.921248969268 , 0.93588055679
	0.949889545294 , 0.962447810601 , 0.97303145761 , 0.981665047202
	0.98772572227 , 0.990268826362 , 0.98975080407 , 0.985143012275
	0.976565690041 , 0.963994500259 , 0.947244857497 , 0.926320875964
	0.901368502029 , 0.873137013018 , 0.841443253538 , 0.807664937995
	0.771305431792 , 0.733812181705 , 0.696243391556 , 0.658588230718
	0.622215002182 , 0.588693827285 , 0.556549843901 , 0.528039103909
	0.503934075555 , 0.484724830921 , 0.470873237129 , 0.462519202205
	},
	{ //Bandpass based on scaled Andre Gunst data -0.5 *2
	-0.0416588758682 , 0.0290052614364 , 0.100279143124 , 0.171697368571
	0.242777302443 , 0.313025848679 , 0.381946687603 , 0.449047844878
	0.513849448188 , 0.575891517633 , 0.634741629366 , 0.690002289101
	0.741317853207 , 0.788380840045 , 0.830937483223 , 0.868792391274
	0.901812194832 , 0.92992808226 , 0.953137147547 , 0.971502499629
	0.985152109431 , 0.994276399403 , 0.999124609205 , 1.0
	0.997253987584 , 0.991279320733 , 0.982502444909 , 0.971375212168
	0.958366115216 , 0.943951236519 , 0.928605111762 , 0.912791710594
	0.896955736122 , 0.881514438227 , 0.866850124385 , 0.853303535593
	0.841168234624 , 0.830686129545 , 0.822044227964 , 0.815372687357
	0.810744195004 , 0.80817467822 , 0.807625312618 , 0.809005763999
	0.812178568912 , 0.816964530829 , 0.823148984002 , 0.830488756082
	0.838719644032 , 0.847564206264 , 0.856739667562 , 0.865965732408
	0.87497210682 , 0.883505538674 , 0.891336201335 , 0.898263264989
	0.90411952369 , 0.908774973202 , 0.912139264475 , 0.914162989188
	0.914837786313 , 0.914195291286 , 0.912304981031 , 0.90927099805
	0.905228064114 , 0.900336618046 , 0.894777332008 , 0.888745176037
	0.882443210921 , 0.876076294553 , 0.86984488661 , 0.863939130747
	0.858533382699 , 0.853781337134 , 0.849811886196 , 0.846725819138
	0.844593445928 , 0.843453198998 , 0.843311237371 , 0.84414204701
	0.845890001377 , 0.848471817772 , 0.85177981881 , 0.855685885195
	0.860045966506 , 0.864705001451 , 0.869502088503 , 0.874275742263
	0.878869070416 , 0.883134710725 , 0.886939377052 , 0.890167877388
	0.892726485108 , 0.894545566296 , 0.895581390486 , 0.895817078691
	0.895262670317 , 0.893954318576 , 0.891952651538 , 0.889340362025
	0.886219113416 , 0.882705869365 , 0.878928772756 , 0.875022712436
	0.871124724976 , 0.867369382699 , 0.863884318289 , 0.860786030625
	0.858176106236 , 0.856137976216 , 0.854734310211 , 0.854005127653
	0.853966682533 , 0.854611152473 , 0.855907136412 , 0.857800938822
	0.860218592781 , 0.863068550301 , 0.866244946823 , 0.869631328387
	0.873104715273 , 0.876539865356 , 0.879813594354 , 0.882809008722
	0.885419510256 , 0.887552439342 , 0.889132235983 , 0.890103013909
	0.890430462538 , 0.890103013909 , 0.889132235983 , 0.887552439342
	0.885419510256 , 0.882809008722 , 0.879813594354 , 0.876539865356
	0.873104715273 , 0.869631328387 , 0.866244946823 , 0.863068550301
	0.860218592781 , 0.857800938822 , 0.855907136412 , 0.854611152473
	0.853966682533 , 0.854005127653 , 0.854734310211 , 0.856137976216
	0.858176106236 , 0.860786030625 , 0.863884318289 , 0.867369382699
	0.871124724976 , 0.875022712436 , 0.878928772756 , 0.882705869365
	0.886219113416 , 0.889340362025 , 0.891952651538 , 0.893954318576
	0.895262670317 , 0.895817078691 , 0.895581390486 , 0.894545566296
	0.892726485108 , 0.890167877388 , 0.886939377052 , 0.883134710725
	0.878869070416 , 0.874275742263 , 0.869502088503 , 0.864705001451
	0.860045966506 , 0.855685885195 , 0.85177981881 , 0.848471817772
	0.845890001377 , 0.84414204701 , 0.843311237371 , 0.843453198998
	0.844593445928 , 0.846725819138 , 0.849811886196 , 0.853781337134
	0.858533382699 , 0.863939130747 , 0.86984488661 , 0.876076294553
	0.882443210921 , 0.888745176037 , 0.894777332008 , 0.900336618046
	0.905228064114 , 0.90927099805 , 0.912304981031 , 0.914195291286
	0.914837786313 , 0.914162989188 , 0.912139264475 , 0.908774973202
	0.90411952369 , 0.898263264989 , 0.891336201335 , 0.883505538674
	0.87497210682 , 0.865965732408 , 0.856739667562 , 0.847564206264
	0.838719644032 , 0.830488756082 , 0.823148984002 , 0.816964530829
	0.812178568912 , 0.809005763999 , 0.807625312618 , 0.80817467822
	0.810744195004 , 0.815372687357 , 0.822044227964 , 0.830686129545
	0.841168234624 , 0.853303535593 , 0.866850124385 , 0.881514438227
	0.896955736122 , 0.912791710594 , 0.928605111762 , 0.943951236519
	0.958366115216 , 0.971375212168 , 0.982502444909 , 0.991279320733
	0.997253987584 , 1.0 , 0.999124609205 , 0.994276399403
	0.985152109431 , 0.971502499629 , 0.953137147547 , 0.92992808226
	0.901812194832 , 0.868792391274 , 0.830937483223 , 0.788380840045
	0.741317853207 , 0.690002289101 , 0.634741629366 , 0.575891517633
	0.513849448188 , 0.449047844878 , 0.381946687603 , 0.313025848679
	0.242777302443 , 0.171697368571 , 0.100279143124 , 0.0290052614364
	},
	{ //Glued together bandpass based on Renting-scaledGunst-deKoning
	0.457000000000 , 0.463127454743 , 0.472220711418 , 0.486950338012
	0.506848752083 , 0.531513162438 , 0.560359234761 , 0.592669860076
	0.627768854647 , 0.664730275035 , 0.702804594295 , 0.741285665919
	0.779117129089 , 0.815658676657 , 0.850258217699 , 0.88217581432
	0.910810641843 , 0.935971774658 , 0.956945368839 , 0.973932695873
	0.986498967923 , 0.995535514678 , 0.999124609205 , 1.000000000000
	0.997253987584 , 0.991279320733 , 0.982502444909 , 0.971375212168
	0.958366115216 , 0.943951236519 , 0.928605111762 , 0.912791710594
	0.896955736122 , 0.881514438227 , 0.866850124385 , 0.853303535593
	0.841168234624 , 0.830686129545 , 0.822044227964 , 0.815372687357
	0.810744195004 , 0.80817467822 , 0.807625312618 , 0.809005763999
	0.812178568912 , 0.816964530829 , 0.823148984002 , 0.830488756082
	0.838719644032 , 0.847564206264 , 0.856739667562 , 0.865965732408
	0.87497210682 , 0.883505538674 , 0.891336201335 , 0.898263264989
	0.90411952369 , 0.908774973202 , 0.912139264475 , 0.914162989188
	0.914837786313 , 0.914195291286 , 0.912304981031 , 0.90927099805
	0.905228064114 , 0.900336618046 , 0.894777332008 , 0.888745176037
	0.882443210921 , 0.876076294553 , 0.86984488661 , 0.863939130747
	0.858533382699 , 0.853781337134 , 0.849811886196 , 0.846725819138
	0.844593445928 , 0.843453198998 , 0.843311237371 , 0.84414204701
	0.845890001377 , 0.848471817772 , 0.85177981881 , 0.855685885195
	0.860045966506 , 0.864705001451 , 0.869502088503 , 0.874275742263
	0.878869070416 , 0.883134710725 , 0.886939377052 , 0.890167877388
	0.892726485108 , 0.894545566296 , 0.895581390486 , 0.895817078691
	0.895262670317 , 0.893954318576 , 0.891952651538 , 0.889340362025
	0.886219113416 , 0.882705869365 , 0.878928772756 , 0.875022712436
	0.871124724976 , 0.867369382699 , 0.863884318289 , 0.860786030625
	0.858176106236 , 0.856137976216 , 0.854734310211 , 0.854005127653
	0.853966682533 , 0.854611152473 , 0.855907136412 , 0.857800938822
	0.860218592781 , 0.863068550301 , 0.866244946823 , 0.869631328387
	0.873104715273 , 0.876539865356 , 0.879813594354 , 0.882809008722
	0.885419510256 , 0.887552439342 , 0.889132235983 , 0.890103013909
	0.890430462538 , 0.890103013909 , 0.889132235983 , 0.887552439342
	0.885419510256 , 0.882809008722 , 0.879813594354 , 0.876539865356
	0.873104715273 , 0.869631328387 , 0.866244946823 , 0.863068550301
	0.860218592781 , 0.857800938822 , 0.855907136412 , 0.854611152473
	0.853966682533 , 0.854005127653 , 0.854734310211 , 0.856137976216
	0.858176106236 , 0.860786030625 , 0.863884318289 , 0.867369382699
	0.871124724976 , 0.875022712436 , 0.878928772756 , 0.882705869365
	0.886219113416 , 0.889340362025 , 0.891952651538 , 0.893954318576
	0.895262670317 , 0.895817078691 , 0.895581390486 , 0.894545566296
	0.892726485108 , 0.890167877388 , 0.886939377052 , 0.883134710725
	0.878869070416 , 0.874275742263 , 0.869502088503 , 0.864705001451
	0.860045966506 , 0.855685885195 , 0.85177981881 , 0.848471817772
	0.845890001377 , 0.84414204701 , 0.843311237371 , 0.843453198998
	0.844593445928 , 0.846725819138 , 0.849811886196 , 0.853781337134
	0.858533382699 , 0.863939130747 , 0.86984488661 , 0.876076294553
	0.882443210921 , 0.888745176037 , 0.894777332008 , 0.900336618046
	0.905228064114 , 0.90927099805 , 0.912304981031 , 0.914195291286
	0.914837786313 , 0.914162989188 , 0.912139264475 , 0.908774973202
	0.90411952369 , 0.898263264989 , 0.891336201335 , 0.883505538674
	0.87497210682 , 0.865965732408 , 0.856739667562 , 0.847564206264
	0.838719644032 , 0.830488756082 , 0.823148984002 , 0.816964530829
	0.812178568912 , 0.809005763999 , 0.807625312618 , 0.80817467822
	0.810744195004 , 0.815372687357 , 0.822044227964 , 0.830686129545
	0.841168234624 , 0.853303535593 , 0.866850124385 , 0.881514438227
	0.896955736122 , 0.912791710594 , 0.928605111762 , 0.943951236519
	0.958366115216 , 0.971375212168 , 0.982502444909 , 0.991279320733
	0.997253987584 , 1.000000000000 , 0.999291656745 , 0.994720326428
	0.986476139458 , 0.973909478082 , 0.957031650928 , 0.936090125123
	0.911487911013 , 0.883416197127 , 0.851988995581 , 0.817449937835
	0.781778231887 , 0.744177644632 , 0.706185554057 , 0.668572666191
	0.632288508848 , 0.597643771659 , 0.566066582016 , 0.537928768881
	0.513338855382 , 0.494006031267 , 0.479737586964 , 0.470877807316
	}
}
;

using namespace casa;

enum CorrelationTypes {None=0,I=1,Q=2,U=3,V=4,RR=5,RL=6,LR=7,LL=8,XX=9,XY=10,YX=11,YY=12}; //found somewhere in AIPS++, don't remember where

//===============>>>  itoa  <<<===============
//ANSI C++ doesn't seem to have a decent function for this, or I'm not aware of it. Need to rename it to IntToStr(), to avoid confusion
std::string itoa(int value, int base=10)
{ // found on http://www.jb.man.ac.uk/~slowe/cpp/itoa.html
  //maybe copyright Robert Jan Schaper, Ray-Yuan Sheu, Rodrigo de Salvo Braz, Wes Garland and John Maloney
  enum { kMaxDigits = 35 };
  std::string buf;
  buf.reserve( kMaxDigits ); // Pre-allocate enough space.
        // check that the base if valid
  if (base < 2 || base > 16) return buf;
  int quotient = value;
        // Translating number to string with base:
  do {
    buf += "0123456789abcdef"[ std::abs( quotient % base ) ];
    quotient /= base;
  } while ( quotient );
        // Append the negative sign for base 10
  if ( value < 0 && base == 10) buf += '-';
  std::reverse( buf.begin(), buf.end() );
  return buf;
}
namespace LOFAR
{
  namespace CS1
  {
    //===============>>>  BandpassCorrector::BandpassCorrector  <<<===============
    /* initialize some meta data and get the datastorage the right size. */
    BandpassCorrector::BandpassCorrector(MS_File* InputMSfile,
                                         int InputWindowSize)
    {
      MSfile           = InputMSfile;
      WindowSize       = InputWindowSize;
      NumAntennae      = (*MSfile).itsNumAntennae;
      NumPairs         = (*MSfile).itsNumPairs;
      NumBands         = (*MSfile).itsNumBands;
      NumChannels      = (*MSfile).itsNumChannels;
      NumPolarizations = (*MSfile).itsNumPolarizations;
      NumTimeslots     = (*MSfile).itsNumTimeslots;
      AntennaNames     = (*MSfile).itsAntennaNames;

      PairsIndex.resize(NumPairs);

      TimeslotData.resize(NumPairs*NumBands);
      for (int i = 0; i < NumPairs*NumBands; i++)
      { TimeslotData[i].resize(NumPolarizations, NumChannels, WindowSize);
      }

      WriteData.resize(NumPairs*NumBands);
      for (int i = 0; i < NumPairs*NumBands; i++)
      { WriteData[i].resize(NumPolarizations, NumChannels);
      }

      int index = 0;
      for (int i = 0; i < NumAntennae; i++)
      { for(int j = i; j < NumAntennae; j++)
        { PairsIndex[index]           = pairii(i, j);
          BaselineIndex[pairii(i, j)] = index++;
        }
      }
      ComputeBaselineLengths();
    }

    //===============>>>  BandpassCorrector::~BandpassCorrector  <<<===============

    BandpassCorrector::~BandpassCorrector()
    {
    }

    //===============>>> BandpassCorrector::ComputeBaselineLengths  <<<===============
    /* compute baseline lengths, and determine the longest one.*/
    void BandpassCorrector::ComputeBaselineLengths()
    {
      MaxBaselineLength = 0.0;
      BaselineLengths.resize(NumPairs);
      //Antenna positions
      MSAntenna antenna     = (*MSfile).antenna();
      ROArrayColumn<Double>  position(antenna, "POSITION");
      for (int i = 0; i < NumAntennae; i++ )
      {
        for (int j = i; j < NumAntennae; j++)
        {
          Vector<Double> p(position(i) - position(j));
          double temp   = sqrt(p(0)*p(0) + p(1)*p(1) + p(2)*p(2));
          BaselineLengths[BaselineIndex[pairii(i, j)]] = temp;
          if (temp > MaxBaselineLength && temp < 3000000) //radius of the Earth in meters? WSRT sometimes has fake telescopes at 3854243
          { MaxBaselineLength = temp;                     // non-existent antenna's can have position (0,0,0)
          }
        }
      }
    }

    //===============>>> BandpassCorrector::ProcessBaselineBand  <<<===============
    /*
    */
    void BandpassCorrector::ProcessBaselineBand(Cube<Complex>* Timeslots,
                                                Matrix<Complex>* Data,
                                                int Position, int fixed)
    {
      for (int i = NumChannels-1; i >= 0; i--)
      {
        for (int j = NumPolarizations-1; j >= 0; j--)
        {
          if (fixed) // we should not do it down here, better is a separate function at the top, but this was easier to implement right now.
          {
            (*Data)(j, i) = (*Timeslots)(j, i, Position) / StaticBandpass[fixed, i];
          }
          else
          {
            double MS = 0.0;
            for (int k = 0; k < WindowSize; k++)
            { //This might be faster in some other way ?
              MS += abs((*Timeslots)(j, i, k));
            }
            double RMS = MS / WindowSize;
            (*Data)(j, i) = (*Timeslots)(j, i, Position) / RMS;
          }
        }
      }
    }
    //===============>>> BandpassCorrector::FlagBaseline  <<<===============
    /* This function iterates over baseline and band and uses FlagBaselineBand() to determine
      for each one if it needs to be flagged. It treats the autocorrelations separately,
      to detect entire malfunctioning telescopes. Finally it writes the flags.
    */
    void BandpassCorrector::ProcessTimeslot(TableIterator* write_iter,
                                            TableIterator* data_iter,
                                            Int Position, int fixed)
    {
      Table         DataTable = (*data_iter).table();
      int           rowcount = DataTable.nrow();
      ROTableVector<Int>     antenna1(DataTable, "ANTENNA1");
      ROTableVector<Int>     antenna2(DataTable, "ANTENNA2");
      ROTableVector<Int>     bandnr  (DataTable, "DATA_DESC_ID");
      ROArrayColumn<Complex> data    (DataTable, "DATA");
      Cube<Complex>          Data(NumPolarizations, NumChannels, rowcount);
      data.getColumn(Data);
      for (int i = 0; i < rowcount; i++)
      {
        int bi    = BaselineIndex[pairii(antenna1(i), antenna2(i))];
        int band  = bandnr(i);
        int index = (band % NumBands) * NumPairs + bi;

        WriteData[index] = Data.xyPlane(i);
      }

      for (int i = 0; i < NumBands; i++)
      {
        for(int j = 0; j < NumAntennae; j++)
        {
          for(int k = j; k < NumAntennae; k++)
          {
            int index = i*NumPairs + BaselineIndex[pairii(j, k)];
            if ((BaselineLengths[BaselineIndex[pairii(j, k)]] < 3000000))
            { //we skip bogus telescopes
              ProcessBaselineBand(&(TimeslotData[index]),
                                  &WriteData[index],
                                  Position, fixed);
            }
          }
        }
        for(int j = 0; j < NumAntennae; j++) //write the data
        {
          for(int k = j; k < NumAntennae; k++)
          {
            int index = i*NumPairs + BaselineIndex[pairii(j, k)];
            Matrix<Complex> writedata = WriteData[index];
            (*MSfile).WriteData(write_iter, &writedata);
            (*write_iter)++;
          }
        }
      }
      (*data_iter)++;
    }

    //===============>>> BandpassCorrector::UpdateTimeslotData  <<<===============
    /* This function reads the visibility data for one timeslot and checks if
      a mosaicing mode is active*/
    /* The datastructure Timeslotdata is rather complex because it flattens
      half-filled antenna x antenna x bands matrix into a vector of NumPairs X bands length. */
    bool BandpassCorrector::UpdateTimeslotData(vector<int>* OldFields,
                                                  vector<int>* OldBands,
                                                  int* TimeCounter,
                                                  Table* TimeslotTable,
                                                  double* Time)
    {
      int           rowcount = (*TimeslotTable).nrow();
      ROTableVector<Int>     antenna1((*TimeslotTable), "ANTENNA1");
      ROTableVector<Int>     antenna2((*TimeslotTable), "ANTENNA2");
      ROTableVector<Int>     bandnr  ((*TimeslotTable), "DATA_DESC_ID");
      ROTableVector<Int>     fieldid ((*TimeslotTable), "FIELD_ID");
      ROTableVector<Double>  time    ((*TimeslotTable), "TIME_CENTROID");//for testing purposes
      ROArrayColumn<Complex> data    ((*TimeslotTable), "DATA");
      Cube<Complex>          Data(NumPolarizations, NumChannels, rowcount);

      data.getColumn(Data); //We're not checking Data.nrow() Data.ncolumn(), assuming all data is the same size.
      (*Time) = time(0);//for testing purposes, might be useful in the future

      bool NewField_or_Frequency = false;

      for (int i = 0; i < rowcount; i++)
      {
        int bi    = BaselineIndex[pairii(antenna1(i), antenna2(i))];
        int field = fieldid(i);
        int band  = bandnr(i);
        int index = (band % NumBands) * NumPairs + bi;
        if ((*TimeCounter) > WindowSize - 1)
        {
          if (field != (*OldFields)[bi]) //pointing mosaicing
          { NewField_or_Frequency = true;
          }
          if (band  != (*OldBands)[index]) //frequency mosaicing
          { NewField_or_Frequency = true;
          }
        }
        (*OldFields)[bi]   = field;
        (*OldBands)[index] = band;

        TimeslotData[index].xyPlane((*TimeCounter) % WindowSize) = Data.xyPlane(i); //TimeslotData is only WindowSize size!
      }
      (*TimeCounter)++; //we want to reset if we detect a gap in DATA_DESC_ID or FIELD. Maybe TIME too???
      return NewField_or_Frequency; //assuming everybody is changing field or frequency at the same time!
    }

    //===============>>> BandpassCorrector::FlagDataOrBaselines  <<<===============
    /* This function iterates over the data per timeslot and uses Flagtimeslot()
      to actually flag datapoints (if flagDatapoints), and entire baselines (if flagRMS)*/
    void BandpassCorrector::CorrectBandpass(bool fixed)
    {
      TableIterator timeslot_iter = (*MSfile).TimeslotIterator();
      TableIterator data_iter     = (*MSfile).TimeslotIterator();
      TableIterator write_iter    = (*MSfile).TimeAntennaIterator();
      int           TimeCounter   = 0;
      double        Time          = 0.0;//for testing purposes
      vector<int>   OldFields(NumPairs);         //to check on multipointing and mosaicing
      vector<int>   OldBands(NumPairs*NumBands); //to check on multifrequency and freq mosaicing

      int           step          = NumTimeslots / 10 + 1; //not exact but it'll do
      int           row           = 0;

      while (!timeslot_iter.pastEnd())
      {
        Table TimeslotTable  = timeslot_iter.table();
        bool  NewFieldorFreq = UpdateTimeslotData(&OldFields,
                                                  &OldBands,
                                                  &TimeCounter,
                                                  &TimeslotTable,
                                                  &Time);
        //cout << "Processing: " << MVTime(Time/(24*3600)).string(MVTime::YMD) << endl; //for testing purposes

        if (TimeCounter == WindowSize - 1)
        { //We have filled WindowSize timeslots and need to flag the first WindowSize/2 timeslots
          for (int position = 0; position < WindowSize/2; position++)
          { ProcessTimeslot(&write_iter, &data_iter, position, fixed);
          }
        }
        if (TimeCounter > WindowSize - 1) //nothing special just falg a timeslot
        { ProcessTimeslot(&write_iter, &data_iter, (TimeCounter + WindowSize/2) % WindowSize, fixed);
        }
        timeslot_iter++;
        if (row++ % step == 0) // to tell the user how much % we have processed,
        { cout << 10*(row/step) << "%" << endl; //not very accurate for low numbers of timeslots, but it'll do for now
        }
        if (timeslot_iter.pastEnd() || NewFieldorFreq)
        { //We still need to flag the last WindowSize/2 timeslots
          for (int position = WindowSize/2 + 1; position < WindowSize; position++)
          { ProcessTimeslot(&write_iter, &data_iter, (TimeCounter + position) % WindowSize, fixed);
          }
          TimeCounter = 0; //reset because we have changed to a new Field or frequency
        }
      }
    }
    //===============>>> BandpassCorrector  <<<===============
  }; // namespace CS1
}; // namespace LOFAR
