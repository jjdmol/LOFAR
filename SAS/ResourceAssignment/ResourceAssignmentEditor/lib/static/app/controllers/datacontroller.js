// $Id$

angular.module('raeApp').factory("dataService", function(){
    var self = this;
    self.tasks = [];
    self.resources = [];
    self.resourceGroups = [];
    self.resourceClaims = [];
    self.resourceGroupClaims = [];
    self.tasktypes = [];
    self.taskstatustypes = [];

    self.taskDict = {};
    self.resourceDict = {};
    self.resourceGroupsDict = {};
    self.resourceClaimDict = {};
    self.resourceGroupClaimDict = {};
    self.resourceIdToGroupIdsDict = {};

    self.resourcesWithClaims = [];

    self.filteredTasks = [];
    self.filteredTaskDict = {};

    return self;
});

var dataControllerMod = angular.module('DataControllerMod', ['ngResource']);

dataControllerMod.controller('DataController',
                            ['$scope', '$http', 'dataService',
                            function($scope, $http, dataService) {
    var self = this;
    self.dataService = dataService;

    function toIdBasedDict(list) {
        var dict = {}
        for(var i = list.length-1; i >=0; i--) {
            var item = list[i];
            dict[item.id] = item;
        }
        return dict;
    };

    function mapResourcesToGroups() {
        var dict = {}
        var resources = self.dataService.resources;
        var resourceGroups = self.dataService.resourceGroups;

        if(resources.length > 0) {
            for(var i = resources.length-1; i >=0; i--)
                dict[resources[i].id] = []

            for(var i = resourceGroups.length-1; i >=0; i--) {
                var group = resourceGroups[i];
                var childResourceIds = group.resourceIds;

                for(var j = childResourceIds.length-1; j >=0; j--) {
                    var childResourceId = childResourceIds[j];
                    dict[childResourceId].push(group.id);
                }
            }
        }

        self.dataService.resourceIdToGroupIdsDict = dict;
    };

    function getTasks() {
        $http.get('/rest/tasks').success(function(result) {
            self.dataService.tasks = result.tasks;
            self.dataService.taskDict = toIdBasedDict(self.dataService.tasks);

            self.dataService.filteredTasks = self.dataService.tasks;
            self.dataService.filteredTaskDict = self.dataService.taskDict;
        });
    };

    function getResources() {
        $http.get('/rest/resourceitems').success(function(result) {
            self.dataService.resources = result.resourceitems;
            self.dataService.resourceDict = toIdBasedDict(self.dataService.resources);
            mapResourcesToGroups();

            getResourceGroupClaims();
        });
    };

    function getResourceClaims() {
        $http.get('/rest/resourceclaims').success(function(result) {
            self.dataService.resourceClaims = result.resourceclaims;
            self.dataService.resourceClaimDict = toIdBasedDict(self.dataService.resourceClaims);
        });
    };

    function getResourceGroups() {
        $http.get('/rest/resourcegroups').success(function(result) {
            self.dataService.resourceGroups = result.resourcegroups;
            self.dataService.resourceGroupsDict = toIdBasedDict(self.dataService.resourceGroups);
            mapResourcesToGroups();

            getResources();
        });
    };

    function getResourceGroupClaims() {
        $http.get('/rest/resourcegroupclaims').success(function(result) {
            self.dataService.resourceGroupClaims = result.resourcegroupclaims;
            self.dataService.resourceGroupClaimDict = toIdBasedDict(self.dataService.resourceGroupClaims);

            setTimeout(function() { getResourceClaims() }, 100);
        });
    };

    function getTaskTypes() {
        $http.get('/rest/tasktypes').success(function(result) {
            self.dataService.tasktypes = result.tasktypes;
        });
    };

    function getTaskStatusTypes() {
        $http.get('/rest/taskstatustypes').success(function(result) {
            self.dataService.taskstatustypes = result.taskstatustypes;
        });
    };

    getTaskTypes();
    getTaskStatusTypes();
    getTasks();
    getResourceGroups();
}
]);
