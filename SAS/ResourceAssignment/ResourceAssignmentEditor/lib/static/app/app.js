// $Id$

var app = angular.module('raeApp',
                         ['DataControllerMod',
                         'GanttControllerMod',
                         'GridControllerMod',
                         'ui.layout',
                         'ui.bootstrap.datetimepicker'
                        ]);

app.config(['$compileProvider', function ($compileProvider) {
    $compileProvider.debugInfoEnabled(false);
}]);
