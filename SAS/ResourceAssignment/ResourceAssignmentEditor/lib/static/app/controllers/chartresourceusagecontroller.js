// $Id:$

var chartResourceUsageControllerMod = angular.module('ChartResourceUsageControllerMod', []).config(['$compileProvider', function($compileProvider) {
    $compileProvider.debugInfoEnabled(false); // Remove debug info (angularJS >= 1.3)
}]);

chartResourceUsageControllerMod.controller('ChartResourceUsageController', ['$scope', 'dataService', function($scope, dataService) {

    Highcharts.setOptions({
        global: {
            useUTC: false
        }
    });

    var self = this;
    self.seriesVisibilityCache = {};

    $scope.dataService = dataService;

    $scope.chartSeries = [];

    $scope.chartConfig = {
        options: {
            chart: {
                type: 'area',
                animation: {
                    duration: 200
                }
            },
            plotOptions: {
                line: {
                    stacking: '',
                    lineWidth: 0,
                    marker:{enabled:false}
                },
                area: {
                    stacking: 'normal',
                    lineWidth: 1,
                    marker: {
                        symbol: 'diamond'
                    }
                },
            }
        },
        xAxis: {
            type: 'datetime',
        },
        yAxis: {
            title: { text: '<unknown>' },
        },
        series: $scope.chartSeries,
        title: {
            text: 'Resource usage'
        },
        credits: {
            enabled: false
        },
        loading: false
    }

    function updateChartLofarTime() {
        var lofarTime = $scope.dataService.lofarTime;

        if(lofarTime.getSeconds() % 5 == 0) {
            $scope.chartConfig.xAxis.plotLines = [{
                width: 3,
                color: '#222222',
                zIndex: 100,
                value: lofarTime.getTime()
            }];
        }
    };

    $scope.$watch('dataService.lofarTime', updateChartLofarTime);

    function updateChartData() {
        var selected_resource_id = $scope.dataService.selected_resource_id;
        if(selected_resource_id === undefined)
            return;

        var resourceDict = $scope.dataService.resourceDict;
        var resourceUsagesDict = $scope.dataService.resourceUsagesDict;
        var numResources = $scope.dataService.resources.length;

        if(numResources == 0) {
            $scope.chartSeries.splice(0, $scope.chartSeries.length);
            $scope.chartConfig.title.text = "No resource available";
            return;
        }

        var resource = resourceDict[selected_resource_id];

        if(!resource || numResources == 0 || !resourceUsagesDict[selected_resource_id]) {
            $scope.chartSeries.splice(0, $scope.chartSeries.length);
            $scope.chartConfig.title.text = "No resource (usages) available";
            return;
        }

        //set title, axis etc
        $scope.chartConfig.title.text = resource.name;
        $scope.chartConfig.yAxis.title.text = resource.units;

        $scope.chartConfig.xAxis.min = $scope.dataService.viewTimeSpan.from.getTime();
        $scope.chartConfig.xAxis.max = $scope.dataService.viewTimeSpan.to.getTime();

        var status_usages = resourceUsagesDict[selected_resource_id].usages;

        //first scan of all statuses and timestamps in usages for this resource
        var statuses = [];
        var timestamps = [];
        for(var status in status_usages) {
            statuses.push(status);

            var usages = status_usages[status];
            for(var usage of usages) {
                timestamps.push(usage.timestamp);
            }
        }

        // the processed statuses are the expected series names, make copy
        var expectedSeriesNames = statuses.slice(0);

        if(timestamps.length > 0) {
            // make timestamps unique
            timestamps = timestamps.filter(function(value, index, arr) { return arr.indexOf(value) == index;})

            //and sort them
            timestamps.sort(function (ts1, ts2) {
                if (ts1 > ts2) return 1;
                if (ts1 < ts2) return -1;
                return 0;
                });

            // loop again over the  usages for this resource
            // loop in predefined status order, so the chart's series are stacked in the correct order
            for(var status of ['conflict', 'claimed', 'allocated']) {
                if(!status_usages.hasOwnProperty(status))
                    continue;

                usage_data = [];

                var usages = status_usages[status];
                var t_idx = 0, t_length = timestamps.length;
                var u_idx = 0, u_length = usages.length;
                var u_min_timestamp = usages[0].timestamp;
                var u_max_timestamp = usages[u_length-1].timestamp;
                while(t_idx < t_length) {
                    var timestamp = timestamps[t_idx];
                    var value = 0;

                    if(u_idx < u_length-1 && timestamp >= usages[u_idx+1].timestamp) {
                        u_idx += 1;
                    }

                    if(u_idx < u_length) {
                        var usage = usages[u_idx];

                        if(timestamp >= u_min_timestamp && timestamp < u_max_timestamp) {
                            value = usage.value;
                        }
                    }
                    usage_data.push([timestamp.getTime(), value]);
                    t_idx += 1;
                }

                //make sure the series are in the right order for proper stacking
                var seriesIdx = $scope.chartSeries.findIndex(function(series) {return series.name == status});
                if(seriesIdx > -1) {
                    $scope.chartSeries.splice(seriesIdx, 1);
                }
                var series = {name: status, type: 'area', step: true, lineWidth:0, marker:{enabled:false}, animation:false };
                series.events = { legendItemClick: function() { self.seriesVisibilityCache[this.name] = !this.visible; }};
                series.visible = self.seriesVisibilityCache.hasOwnProperty(series.name) ? self.seriesVisibilityCache[series.name] : true;
                $scope.chartSeries.push(series);

                series.data = usage_data;

                switch(status) {
                    case 'claimed': series.color = '#ffa64d'; break;
                    case 'conflict': series.color = '#ff0000'; break;
                    case 'allocated': series.color = '#66ff66'; break;
                }
            }

            //plot area for resource misc_used capacity
            //make sure it is the last of the 'area' series so it is at the bottom of the stacked area charts
            var misc_used_cap_series_idx = $scope.chartSeries.findIndex(function(series) {return series.name == 'misc used capacity'});
            if(misc_used_cap_series_idx > -1) {
                $scope.chartSeries.splice(misc_used_cap_series_idx, 1);
            }

            var misc_used_capacity = resourceUsagesDict[selected_resource_id].misc_used_capacity;
            if(misc_used_capacity > 0) {
                series = $scope.chartSeries.find(function(series) {return series.name == 'misc used capacity'});
                if(!series) {
                    series = {name: 'misc used capacity', type: 'area', color: '#aaaaff', lineWidth:1, marker:{enabled:false}, dashStyle:'Dash', animation:false };
                    series.events = { legendItemClick: function() { self.seriesVisibilityCache[this.name] = !this.visible; }};
                    $scope.chartSeries.push(series);
                }
                series.visible = self.seriesVisibilityCache.hasOwnProperty(series.name) ? self.seriesVisibilityCache[series.name] : true;
                series.data = timestamps.map(function(t) { return [t.getTime(), misc_used_capacity]; });
                expectedSeriesNames.push('misc used capacity');
            }

            //plot horizontal line for resource total capacity
            series = $scope.chartSeries.find(function(series) {return series.name == 'total capacity'});
            if(!series) {
                series = {name: 'total capacity', type: 'line', color: '#ff0000', lineWidth:3, marker:{enabled:false}};
                series.events = { legendItemClick: function() { self.seriesVisibilityCache[this.name] = !this.visible; }};
                $scope.chartSeries.push(series);
            }
            series.visible = self.seriesVisibilityCache.hasOwnProperty(series.name) ? self.seriesVisibilityCache[series.name] : true;
            series.data = [[timestamps[0].getTime(), resource.total_capacity],
                           [timestamps[timestamps.length-1].getTime(), resource.total_capacity]]
            expectedSeriesNames.push('total capacity');

            //plot horizontal line for resource used capacity
            series = $scope.chartSeries.find(function(series) {return series.name == 'used capacity'});
            if(!series) {
                series = {name: 'used capacity', type: 'line', color: '#ff9966', lineWidth:3, marker:{enabled:false}, dashStyle:'Dash'};
                series.events = { legendItemClick: function() { self.seriesVisibilityCache[this.name] = !this.visible; }};
                $scope.chartSeries.push(series);
            }
            series.visible = self.seriesVisibilityCache.hasOwnProperty(series.name) ? self.seriesVisibilityCache[series.name] : true;
            series.data = [[timestamps[0].getTime(), resource.used_capacity],
                           [timestamps[timestamps.length-1].getTime(), resource.used_capacity]]
            expectedSeriesNames.push('used capacity');
        }


        for(var i = $scope.chartSeries.length-1; i >= 0; i--) {
            if(!expectedSeriesNames.find(function(s) { return s == $scope.chartSeries[i].name;})) {
                self.seriesVisibilityCache[$scope.chartSeries.name] = $scope.chartSeries.visible;
                $scope.chartSeries.splice(i, 1);
                i = $scope.chartSeries.length-1;
            }
        }
    };

    $scope.$watch('dataService.selected_resource_id', updateChartData);
    $scope.$watch('dataService.resources', updateChartData, true);
    $scope.$watch('dataService.resourceUsagesDict', updateChartData, true);
    $scope.$watch('dataService.viewTimeSpan', updateChartData, true);
}
]);
