// $Id$

var app = angular.module('raeApp',
                         ['DataControllerMod',
                         'GanttResourceControllerMod',
                         'GanttProjectControllerMod',
                         'GridControllerMod',
                         'ui.layout',
                         'ui.bootstrap.datetimepicker',
                         'ui.bootstrap.tabs'
                        ]);

app.config(['$compileProvider', function ($compileProvider) {
    $compileProvider.debugInfoEnabled(false);
}]);
